package com.example.smarthome.coap;

import com.example.smarthome.coap.DeviceRegistry;
import com.example.smarthome.db.DatabaseManager;
import com.example.smarthome.model.DeviceModel;

import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.server.resources.CoapExchange;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class CoapRegistrationServer extends CoapServer {

    public CoapRegistrationServer() {
        add(new RegistrationResource());
        add(new DiscoveryResource());
    }

    private static class RegistrationResource extends CoapResource {
        public RegistrationResource() {
            super("registration");
        }

        @Override
        public void handlePOST(CoapExchange exchange) {
            System.out.println("Received registration request");
            String type = exchange.getRequestText(); 
            String ip = exchange.getSourceAddress().getHostAddress();
            int port = exchange.getSourcePort();

            DeviceRegistry.registerDevice(type, ip);
            try{
                DatabaseManager.insertSensor(type, ip, port);
            }
            catch (Exception e) {
                System.err.println("[REGISTRATION] Error during registration: " + e.getMessage());
                DeviceRegistry.unregisterDevice(type); // consistency between registry and database
                exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR, "Failed");
                return;
            }
            
            exchange.respond(CoAP.ResponseCode.CREATED, "Registered".getBytes(StandardCharsets.UTF_8));
        }

        @Override
        public void handleDELETE(CoapExchange exchange) {
            String type = exchange.getRequestText();

            if (DeviceRegistry.contains(type)) {
                DeviceRegistry.unregisterDevice(type);
                exchange.respond(CoAP.ResponseCode.DELETED, "Unregistered");
            } else {
                exchange.respond(CoAP.ResponseCode.BAD_REQUEST, "Device not found");
            }
        }
    }

    private static class DiscoveryResource extends CoapResource {
        public DiscoveryResource() {
            super("sensors-discovery");
        }

        @Override
        public void handleGET(CoapExchange exchange) {
            System.out.println("Discovery request received");
        
            try{
                List<DeviceModel> devices = DatabaseManager.getAllSensors();
                if(devices.isEmpty()) {
                    exchange.respond(CoAP.ResponseCode.CONTENT, "No devices found".getBytes(StandardCharsets.UTF_8));
                    return;
                }

                List<DeviceModel> sensors = new ArrayList<DeviceModel>();
                for (DeviceModel device : devices) {
                    if (device.getName() != null && device.getName().toLowerCase().contains("sensor")) {
                        sensors.add(device);
                    }
                }

                if(sensors.isEmpty()) {
                    exchange.respond(CoAP.ResponseCode.CONTENT, "No sensors found".getBytes(StandardCharsets.UTF_8));
                    return;
                }

                for (DeviceModel device : sensors) {
                    StringBuilder response = new StringBuilder();
                    response.append(device.getName())
                            .append("@")
                            .append(device.getIp())
                            .append("-")
                            .append(device.getPort());
                    System.out.println("[DISCOVERY] Device found: " + response);
                    exchange.respond(CoAP.ResponseCode.CONTENT, response.toString().getBytes(StandardCharsets.UTF_8));
                }
                
            }
            catch (Exception e) {
                System.err.println("[DISCOVERY] Error during discovery: " + e.getMessage());
                exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR, "Failed");
                return;
            }
        }
    }
}
