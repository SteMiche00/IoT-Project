package com.example.smarthome.coap;

import com.example.smarthome.coap.DeviceRegistry;
import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;


import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class SensorPoller {
    private final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    public void startPolling() {
        scheduler.scheduleAtFixedRate(this::pollSensors, 5, 10, TimeUnit.SECONDS);
        System.out.println("[POLL] Poller avviato.");
    }

    private void pollSensors() {
        Map<String, String> devices = DeviceRegistry.getAll();

        if (devices.isEmpty()) {
            System.out.println("[POLL] Nessun dispositivo registrato.");
            return;
        }

        for (Map.Entry<String, String> entry : devices.entrySet()) {
            String name = entry.getKey();
            String ip = entry.getValue();

            String path = null;
            switch (name) {
                case "temperature" : path = "sensor/temperature"; break;
                case "light"       : path = "sensor/light"; break;
                case "huidity"     : path = "sensor/humidity"; break;
                default            : path = null; break;
            }

            if (path == null) continue;

            String uri = "coap://" + ip + "/" + path;
            CoapClient client = new CoapClient(uri);

            try {
                CoapResponse response = client.get();
                if (response == null) {
                    System.err.println("[POLL] Nessuna risposta da " + name);
                    continue;
                }

                String payload = response.getResponseText();
                System.out.printf("[POLL] %s @ %s = %s%n", name, ip, payload);
            } catch (Exception e) {
                System.err.printf("[POLL] Errore con %s @ %s: %s%n", name, ip, e.getMessage());
            }
        }
    }
}
