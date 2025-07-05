package com.example.smarthome.coap;

import com.example.smarthome.coap.DeviceRegistry;
import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.server.resources.CoapExchange;

import java.nio.charset.StandardCharsets;

public class CoapRegistrationServer extends CoapServer {

    public CoapRegistrationServer() {
        add(new RegistrationResource());
    }

    private static class RegistrationResource extends CoapResource {
        public RegistrationResource() {
            super("registration");
        }

        @Override
        public void handlePOST(CoapExchange exchange) {
            String type = exchange.getRequestText(); 
            String ip = exchange.getSourceAddress().getHostAddress();

            DeviceRegistry.registerDevice(type, ip);
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
}
