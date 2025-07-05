package com.example.smarthome;

import com.example.smarthome.coap.CoapRegistrationServer;
import com.example.smarthome.coap.SensorPoller;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.server.resources.CoapExchange;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class SmartHomeCollector {
    public static void main(String[] args) {
        System.out.println("=== SMART HOME COLLECTOR ===");

        // 1. Avvia il server CoAP principale
        CoapRegistrationServer coapServer = new CoapRegistrationServer();
        coapServer.start();
        System.out.println("[INFO] CoAP server started on port 5683");

       

        // 2. Avvia sensori finti
        startFakeSensor("temperature", 5684, "sensor/temperature", "22");
        startFakeSensor("light", 5685, "sensor/light", "650");
        startFakeSensor("humidity", 5686, "sensor/presence", "1");
        startFakeActuator("light", 5687, "actuator/light");
        startFakeActuator("temperature", 5688, "actuator/temperature");

        SensorPoller sensorPoller = new SensorPoller();
        sensorPoller.startPolling();
        
        // 3. Mostra comandi
        printHelp();

        // 4. CLI utente
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String input;

        while (true) {
            try {
                System.out.print("> ");
                input = br.readLine().trim();

                switch (input) {
                    case "exit":
                        System.out.println("Chiudo il server...");
                        coapServer.stop();
                        return;
                    case "help":
                        printHelp();
                        break;
                    case "get_air_quality":
                        System.out.println("Simulazione: qualità aria = 400 ppm CO2");
                        break;
                    case "set_light_color RED":
                        System.out.println("Luce impostata a ROSSO");
                        break;
                    default:
                        System.out.println("Comando non riconosciuto. Digita 'help'");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    // 👇 Simula un sensore CoAP che espone una risorsa GET
    private static void startFakeSensor(String type, int port, String path, String value) {
        CoapServer sensorServer = new CoapServer(port);
        String[] parts = path.split("/");

        sensorServer.add(new CoapResource(parts[0])
            .add(new CoapResource(parts[1]) {
                @Override
                public void handleGET(CoapExchange exchange) {
                    exchange.respond(value);
                }
            })
        );
        sensorServer.start();
        System.out.println("[FAKE SENSOR] '" + type + "' attivo su porta " + port);

        // Auto-registrazione
        // Auto-registrazione
        CoapClient client = new CoapClient("coap://localhost:5683/registration");
        try {
            client.post(type, MediaTypeRegistry.TEXT_PLAIN);
            System.out.println("[REGISTER] Sensore '" + type + "' registrato con successo.");
        } catch (Exception e) {
            System.err.println("[REGISTER ERROR] Errore nella registrazione del sensore '" + type + "': " + e.getMessage());
        }

    }

    // 👇 Simula un attuatore CoAP che espone risorsa PUT/GET
    private static void startFakeActuator(String type, int port, String path) {
        CoapServer actuatorServer = new CoapServer(port);
        String[] parts = path.split("/");

        actuatorServer.add(new CoapResource(parts[0])
            .add(new CoapResource(parts[1]) {
                String state = "OFF";

                @Override
                public void handlePUT(CoapExchange exchange) {
                    state = exchange.getRequestText();
                    System.out.println("[ACTUATOR] " + type + " set to " + state);
                    exchange.respond(CoAP.ResponseCode.CHANGED, "OK");
                }

                @Override
                public void handleGET(CoapExchange exchange) {
                    exchange.respond(state);
                }
            })
        );
        actuatorServer.start();
        System.out.println("[FAKE ACTUATOR] '" + type + "' attivo su porta " + port);

        // Auto-registrazione
        // Auto-registrazione
        CoapClient client = new CoapClient("coap://localhost:5683/registration");
        try {
            client.post(type, MediaTypeRegistry.TEXT_PLAIN);
            System.out.println("[REGISTER] Sensore '" + type + "' registrato con successo.");
        } catch (Exception e) {
            System.err.println("[REGISTER ERROR] Errore nella registrazione del sensore '" + type + "': " + e.getMessage());
        }

    }

    private static void printHelp() {
        System.out.println("\nComandi disponibili:");
        System.out.println("  help                  - mostra i comandi");
        System.out.println("  get_air_quality       - mostra qualità aria");
        System.out.println("  set_light_color RED   - imposta colore luce");
        System.out.println("  exit                  - chiude il programma");
    }
}
