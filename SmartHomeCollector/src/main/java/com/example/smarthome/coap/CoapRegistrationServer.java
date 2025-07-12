package com.example.smarthome.coap;

import com.example.smarthome.coap.DeviceRegistry;
import com.example.smarthome.db.DatabaseManager;
import com.example.smarthome.model.DeviceModel;

import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.server.resources.CoapExchange;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

public class CoapRegistrationServer extends CoapServer {

   
    public CoapRegistrationServer() {
        add(new RegistrationResource());
        add(new DiscoveryResource());
    }

    private static class RegistrationResource extends CoapResource {
        public RegistrationResource() {
            super("registration");
        }

        private static final Logger LOGGER = Logger.getLogger(RegistrationResource.class.getName());

        @Override
        public void handlePOST(CoapExchange exchange) {
            LOGGER.info("Received registration request");
            String type = exchange.getRequestText(); 
            String ip = exchange.getSourceAddress().getHostAddress();
            int port = exchange.getSourcePort();

            DeviceRegistry.registerDevice(type, ip);
            try{
                DatabaseManager.insertDevice(type, ip, port);
            }
            catch (Exception e) {
                LOGGER.severe("[REGISTRATION] Error during registration: " + e.getMessage());
                DeviceRegistry.unregisterDevice(type); // consistency between registry and database
                exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR, "Failed");
                return;
            }
            if(type.contains("sensor")) {
                String[] parts = type.split("_");
                LOGGER.info(String.format("[REGISTRATION] Starting sensor observing for " + "coap://" + ip + "/sensors/" + parts[1]));
                DeviceObserverManager.observeSensor(type, ip, port);
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

        private static final Logger LOGGER = Logger.getLogger(DiscoveryResource.class.getName());

        @Override
        public void handleGET(CoapExchange exchange) {
            LOGGER.info("Discovery request received");

            String query = exchange.getRequestOptions().getUriQuery().toString();
            String requestedType = exchange.getRequestOptions().getUriQuery().stream()
                    .filter(q -> q.startsWith("type="))
                    .map(q -> q.substring(5))
                    .findFirst()
                    .orElse(null);

            if (requestedType == null) {
                exchange.respond(CoAP.ResponseCode.BAD_REQUEST, "Missing 'type' query parameter");
                return;
            }

            try {
                List<DeviceModel> devices = DatabaseManager.getAllDevices();
                if (devices.isEmpty()) {
                    exchange.respond(CoAP.ResponseCode.CONTENT, "No devices found");
                    return;
                }

                for (DeviceModel device : devices) {
                    if (device.getName() != null &&
                        device.getName().toLowerCase().contains("sensor") &&
                        device.getName().toLowerCase().contains(requestedType.toLowerCase())) {

                        String response = device.getName() + "@" + device.getIp() + "-" + device.getPort();
                        LOGGER.info("[DISCOVERY] Responding with: " + response);
                        exchange.respond(CoAP.ResponseCode.CONTENT, response);
                        return;
                    }
                }

                exchange.respond(CoAP.ResponseCode.NOT_FOUND, "No sensor of type '" + requestedType + "' found");

            } catch (Exception e) {
                LOGGER.severe("[DISCOVERY] Error during discovery: " + e.getMessage());
                exchange.respond(CoAP.ResponseCode.INTERNAL_SERVER_ERROR, "Failed");
            }
        }
    }
}
