  /* 1. Conéctate al punto de acceso 
    2. Apunta tu navegador web a http://192.168.4.1/H para encender el optoacoplador o http://192.168.4.1/L para apagarlo */

  #include <WiFi.h>
  #include <NetworkClient.h>
  #include <WiFiAP.h>

  // Define los pines donde están conectados el optoacoplador y los pulsadores
  const int optoPin = 23;
    

  // Configura estos con tus credenciales deseadas.
  const char *ssid = "Motor";
  const char *password = "Motorcito123";

  NetworkServer server(80);
  bool estadoOpto = false; // Variable para rastrear el estado del optoacoplador

  void setup() {
    pinMode(optoPin, OUTPUT); // Configura el pin del optoacoplador como salida
    digitalWrite(optoPin, 0); // Inicia con el optoacoplador apagado

    pinMode(botonEncendidoPin, INPUT_PULLUP); // Configura el pin del pulsador de encendido como entrada con resistencia pull-up
    pinMode(botonApagadoPin, INPUT_PULLUP); // Configura el pin del pulsador de apagado como entrada con resistencia pull-up

    Serial.begin(115200);
    Serial.println();
    Serial.println("Configurando punto de acceso...");

    // Crea un punto de acceso WiFi con las credenciales proporcionadas
    if (!WiFi.softAP(ssid, password)) {
      log_e("Falló la creación del punto de acceso.");
      while (1);
    }
    IPAddress myIP = WiFi.softAPIP(); // Obtén la dirección IP del punto de acceso
    Serial.print("Dirección IP del AP: ");
    Serial.println(myIP);
    server.begin(); // Inicia el servidor

    Serial.println("Servidor iniciado");
  }

  void loop() {
    // Verifica el estado de los pulsadores
    bool estadoBotonEncendido = digitalRead(botonEncendidoPin);
    bool estadoBotonApagado = digitalRead(botonApagadoPin);

    if (estadoBotonEncendido == 0) {
      estadoOpto = true; // Enciende el optoacoplador
      digitalWrite(optoPin, 1); // Actualiza el estado del optoacoplador
    }
    if (estadoBotonApagado == 0) {
      estadoOpto = false; // Apaga el optoacoplador
      digitalWrite(optoPin, 0); // Actualiza el estado del optoacoplador
    }

    NetworkClient client = server.accept();  // Escucha a los clientes entrantes

    if (client) {                     // Si hay un cliente,
      Serial.println("Nuevo Cliente.");  // Imprime un mensaje en el puerto serial
      String currentLine = "";        // Crea un String para almacenar datos entrantes del cliente
      while (client.connected()) {    // Bucle mientras el cliente esté conectado
        if (client.available()) {     // Si hay bytes para leer del cliente,
          char c = client.read();     // Lee un byte, luego
          Serial.write(c);            // Imprímelo en el monitor serial
          if (c == '\n') {            // Si el byte es un carácter de nueva línea

            // Si la línea actual está en blanco, obtuviste dos caracteres de nueva línea seguidos.
            // Eso es el fin de la solicitud HTTP del cliente, así que envía una respuesta:
            if (currentLine.length() == 0) {
              // Las cabeceras HTTP siempre comienzan con un código de respuesta (por ejemplo, HTTP/1.1 200 OK)
              // y un tipo de contenido para que el cliente sepa qué viene, luego una línea en blanco:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();

              // El contenido de la respuesta HTTP sigue a la cabecera:
              client.print("<!DOCTYPE html><html><head><title>Controlar Optoacoplador</title>");
              client.print("<style>body { display: flex; flex-direction: column; justify-content: center; align-items: center; height: 100vh; margin: 0; font-family: Arial, sans-serif; }");
              client.print("#status { font-size: 24px; margin-bottom: 20px; }"); // Estilo para el estado
              client.print("button { font-size: 40px; padding: 20px 40px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; }"); // Estilo para botones
              client.print(".on { background-color: green; color: white; }"); // Estilo para botón ON
              client.print(".off { background-color: red; color: white; }</style></head><body>"); // Estilo para botón OFF
              client.print("<div id=\"status\">Estado: ");
              client.print(estadoOpto ? "ENCENDIDO" : "APAGADO"); // Muestra el estado actual del optoacoplador
              client.print("</div>");
              client.print("<a href=\"/H\"><button class=\"on\">ENCENDER</button></a>"); // Botón para encender el optoacoplador
              client.print("<a href=\"/L\"><button class=\"off\">APAGAR</button></a>"); // Botón para apagar el optoacoplador
              client.print("</body></html>");

              // La respuesta HTTP termina con otra línea en blanco:
              client.println();
              // Sal del bucle while:
              break;
            } else {  // Si obtuviste una nueva línea, entonces limpia currentLine:
              currentLine = "";
            }
          } else if (c != '\r') {  // Si obtuviste cualquier cosa menos un carácter de retorno de carro,
            currentLine += c;      // Agrégala al final de currentLine
          }

          // Comprueba si la solicitud del cliente fue "GET /H" o "GET /L":
          if (currentLine.endsWith("GET /H")) {
            digitalWrite(optoPin, 1);  // GET /H enciende el optoacoplador
            estadoOpto = true; // Actualiza el estado
          }
          if (currentLine.endsWith("GET /L")) {
            digitalWrite(optoPin, 0);  // GET /L apaga el optoacoplador
            estadoOpto = false; // Actualiza el estado
          }
        }
      }
      // Cierra la conexión:
      client.stop();
      Serial.println("Cliente desconectado.");
    }
  }
