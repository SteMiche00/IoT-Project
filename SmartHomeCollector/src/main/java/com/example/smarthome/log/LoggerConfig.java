package com.example.smarthome.log;

import java.io.IOException;
import java.util.logging.*;

public class LoggerConfig {
    public static void setup() {
        Logger logger = Logger.getLogger("");
        logger.setLevel(Level.INFO);

        // Rimuove i log nel terminale
        Handler[] handlers = logger.getHandlers();
        for (Handler handler : handlers) {
            logger.removeHandler(handler);
        }

        try {
            FileHandler fileHandler = new FileHandler("smarthome.log");
            fileHandler.setFormatter(new SimpleFormatter()); // testo semplice
            logger.addHandler(fileHandler);
        } catch (IOException e) {
            System.err.println("Failed to create log file handler: " + e.getMessage());
        }
    }
}
