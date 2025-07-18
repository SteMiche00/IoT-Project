package com.example.smarthome.coap;

import java.util.HashMap;
import java.util.Map;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;

import com.example.smarthome.db.DatabaseManager;

public class DeviceObserverManager {
    private static final Map<String, CoapObserveRelation> relations = new HashMap<>();
    private static final java.util.logging.Logger LOGGER = java.util.logging.Logger.getLogger(DeviceObserverManager.class.getName());
    public static void observeNode(String type, String ip, int port) {
        String[] parts = type.split("_");
        String uri;
        if(parts[0].equals("actuator"))
            uri = "coap://[" + ip + "]/actuators/" + parts[1].trim() + "_status";
        else
            uri = "coap://[" + ip + "]/sensors/" + parts[1].trim();

        CoapClient client = new CoapClient(uri);
        LOGGER.info("[OBSERVER] Observing " + uri);

        CoapObserveRelation relation = client.observe(new CoapHandler() {
            @Override
            public void onLoad(CoapResponse response) {
                String payload = response.getResponseText();
                LOGGER.info(String.format("[OBSERVER] Node %s @ %s:%d reported: %s%n", type, ip, port, payload));
                DatabaseManager.insertNodeData(type, Double.parseDouble(payload));
            }

            @Override
            public void onError() {
                LOGGER.severe("[OBSERVER] Error observing node " + type);
            }
        });

        relations.put(type, relation); 
    }
}
