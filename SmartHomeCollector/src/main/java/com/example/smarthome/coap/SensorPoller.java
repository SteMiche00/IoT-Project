package com.example.smarthome.coap;

import com.example.smarthome.coap.DeviceRegistry;
import org.eclipse.californium.core.CoapClient;

import java.time.LocalDateTime;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class SensorPoller {
    private final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    public void startPolling() {
        scheduler.scheduleAtFixedRate(this::pollSensors, 0, 10, TimeUnit.SECONDS);
    }

    private void pollSensors() {
        for (Map.Entry<String, String> entry : DeviceRegistry.getAll().entrySet()) {
            String type = entry.getKey();
            String ip = entry.getValue();

            String path = null;
            switch (type) {
                case "temperature":
                    path = "sensor/temperature";
                    break;
                case "light":
                    path = "sensor/light";
                    break;
                case "humidity":
                    path = "sensor/humidity";
                    break;
}
            if (path == null) continue;

            String uri = "coap://" + ip + "/" + path;
            CoapClient client = new CoapClient(uri);

            try {
                String payload = client.get().getResponseText();
                System.out.printf("[SensorPoller] %s @ %s = %s%n", type, ip, payload);
            } catch (Exception e) {
                System.err.printf("[SensorPoller] Errore con %s @ %s: %s%n", type, ip, e.getMessage());
            }
        }
    }
}
