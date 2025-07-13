package com.example.smarthome.commands;

import java.util.Locale;
import java.util.Scanner;
import java.util.logging.Logger;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;

import com.example.smarthome.db.DatabaseManager;

public class ThresholdManager {
    private final static java.util.logging.Logger LOGGER = java.util.logging.Logger.getLogger(ThresholdManager.class.getName());

    private static final Scanner scanner = new Scanner(System.in);

    public static void setThresholds() {
        System.out.println("Choose actuator to configure:");
        System.out.println("1. Light");
        System.out.println("2. Temperature");
        System.out.print("> ");

        String actuatorType = null;
        String input = scanner.nextLine();

        switch (input) {
            case "1":
                actuatorType = "actuator_light";
                break;
            case "2":
                actuatorType = "actuator_temp";
                break;
            default:
                System.out.println("Invalid selection.");
                return;
        }

        try {
            System.out.print("Enter minimum threshold value: ");
            double minValue = Double.parseDouble(scanner.nextLine());

            double maxValue = 0;
            if(actuatorType.equals("actuator_temp")) {
                System.out.print("Enter maximum threshold value: ");
                maxValue = Double.parseDouble(scanner.nextLine());
            }
           
            LOGGER.info(String.format("Setting threshold for %s: min=%.2f, max=%.2f", actuatorType, minValue, maxValue));

            String uri = DatabaseManager.getNodeUri(actuatorType);
            LOGGER.info("Using URI: " + uri);
            CoapClient client = new CoapClient(uri);
            String payload = actuatorType.equals("actuator_temp") ? String.format(Locale.US,"min=%.2f&max=%.2f", minValue, maxValue) : String.format(Locale.US,"min=%.2f", minValue);
            LOGGER.info("Payload: " + payload);
            CoapResponse response = client.post(payload, MediaTypeRegistry.TEXT_PLAIN);

            if (response != null && response.isSuccess()) {
                System.out.println("Threshold updated successfully.");
            } else {
                System.out.println("Failed to update threshold.");
            }

        } catch (NumberFormatException e) {
            System.out.println("Invalid number format.");
        } catch (Exception e) {
            System.out.println("Error setting threshold: " + e.getMessage());
        }
    }

    public static void getThresholds(){
        System.out.println("Choose actuator to configure:");
        System.out.println("1. Light");
        System.out.println("2. Temperature");
        System.out.print("> ");

        String actuatorType = null;
        String input = scanner.nextLine();

        switch (input) {
            case "1":
                actuatorType = "actuator_light";
                break;
            case "2":
                actuatorType = "actuator_temp";
                break;
            default:
                System.out.println("Invalid selection.");
                return;
        }
        try{
            String uri = DatabaseManager.getNodeUri(actuatorType);
            LOGGER.info("Using URI: " + uri);
            CoapClient client = new CoapClient(uri);
            CoapResponse response = client.get();
            LOGGER.info("Response: " + (response != null ? response.getResponseText() : "null"));
            if (response != null && response.isSuccess()) {
                String payload = response.getResponseText();
                if(actuatorType.equals("actuator_temp")){
                String parts[] = payload.split("&");
                String minThreshold = parts[0].split("=")[1];
                String maxThreshold = parts[1].split("=")[1];
                System.out.println("Current thresholds for " + actuatorType + ": " + 
                                   "Min = " + minThreshold + ", Max = " + maxThreshold);
                }
                else {
                    String minThreshold = payload.split("=")[1];
                    System.out.println("Current threshold for " + actuatorType + ": " + 
                                       "Min = " + minThreshold);
                }
            } else {
                System.out.println("Failed to retrieve thresholds.");
            }
        } catch (Exception e) {
            System.out.println("Error retrieving thresholds: " + e.getMessage());
        }

    }
}
