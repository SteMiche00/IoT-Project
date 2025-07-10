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

    public static void observeSensor(String type, String ip, int port) {
        String[] parts = type.split("_");
        String uri = "coap://[" + ip + "]/sensors/" + parts[1].trim();

        CoapClient client = new CoapClient(uri);
        System.out.println("[OBSERVER] Observing " + uri);

        CoapObserveRelation relation = client.observe(new CoapHandler() {
            @Override
            public void onLoad(CoapResponse response) {
                String payload = response.getResponseText();
                System.out.printf("[OBSERVER] Sensor %s @ %s:%d reported: %s%n", type, ip, port, payload);
                DatabaseManager.insertSensorData(type, Double.parseDouble(payload));
            }

            @Override
            public void onError() {
                System.err.println("[OBSERVER] Error observing sensor " + type);
            }
        });

        relations.put(type, relation); // keep it alive
    }
}
