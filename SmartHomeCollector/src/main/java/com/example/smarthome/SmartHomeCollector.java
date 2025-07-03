package main.java.com.example.smarthome;

import main.java.com.example.smarthome.coap.CoapRegistrationServer;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class SmartHomeCollector {
    public static void main(String[] args) {
        System.out.println("=== SMART HOME CONTROLLER ===");

        // 1. Avvia il server CoAP
        CoapRegistrationServer coapServer = new CoapRegistrationServer();
        coapServer.start();
        System.out.println("CoAP server started on port 5683");

        // 2. Mostra i comandi
        printHelp();

        // 3. Gestione CLI utente
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

    private static void printHelp() {
        System.out.println("\nComandi disponibili:");
        System.out.println("  help                  - mostra i comandi");
        System.out.println("  get_air_quality       - mostra qualità aria");
        System.out.println("  set_light_color RED   - imposta colore luce");
        System.out.println("  exit                  - chiude il programma");
    }
}
