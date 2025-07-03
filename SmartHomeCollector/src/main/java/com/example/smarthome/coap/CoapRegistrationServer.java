package com.example.smarthome.coap;

import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.CoapClient;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class CoapRegistrationServer extends CoapServer {

    private final Map<String, String> registeredDevices = new HashMap<>(); // tipo -> ip

    public CoapRegistrationServer() {
        add(new RegistrationResource());
    }

    class RegistrationResource extends CoapResource {
        public RegistrationResource() {
            super("registration");
        }

        @Override
        public void handlePOST(CoapExchange exchange) {
            String deviceType = exchange.getRequestText();
            String ip = exchange.getSourceAddress().getHostAddress();
            registeredDevices.put(deviceType, ip);
            System.out.println("Registrato: " + deviceType + " da " + ip);
            exchange.respond(CoAP.ResponseCode.CREATED, "Registered".getBytes(StandardCharsets.UTF_8));
        }

        @Override
        public void handleDELETE(CoapExchange exchange) {
            String deviceType = exchange.getRequestText();
            if (registeredDevices.remove(deviceType) != null) {
                System.out.println("Dispositivo rimosso: " + deviceType);
                exchange.respond(CoAP.ResponseCode.DELETED, "Unregistered");
            } else {
                exchange.respond(CoAP.ResponseCode.BAD_REQUEST, "Device not found");
            }
        }
    }

    // Metodi per interagire con i dispositivi registrati

    public Integer getTemperature() {
        return getIntFromDevice("temperature", "sensor/temperature");
    }

    public Integer getLuminosity() {
        return getIntFromDevice("luminosity", "sensor/light");
    }

    public Boolean isSomeonePresent() {
        return getIntFromDevice("presence", "sensor/presence") == 1;
    }

    public boolean setACState(boolean on) {
        return sendCommandToDevice("ac", "actuator/ac", on ? "ON" : "OFF");
    }

    public boolean setLightColor(String color) {
        return sendCommandToDevice("light", "actuator/light", color);
    }

    private Integer getIntFromDevice(String deviceKey, String path) {
        String ip = registeredDevices.get(deviceKey);
        if (ip == null) return null;

        CoapClient client = new CoapClient("coap://" + ip + "/" + path);
        try {
            String payload = client.get().getResponseText();
            return Integer.parseInt(payload);
        } catch (Exception e) {
            return null;
        }
    }

    private boolean sendCommandToDevice(String deviceKey, String path, String command) {
        String ip = registeredDevices.get(deviceKey);
        if (ip == null) return false;

        CoapClient client = new CoapClient("coap://" + ip + "/" + path);
        try {
            client.put(command, CoAP.MediaTypeRegistry.TEXT_PLAIN);
            return true;
        } catch (Exception e) {
            return false;
        }
    }
}
