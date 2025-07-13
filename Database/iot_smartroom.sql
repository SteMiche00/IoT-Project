-- MySQL dump 10.13  Distrib 8.0.42, for Linux (x86_64)
--
-- Host: localhost    Database: iot_smarthome
-- ------------------------------------------------------
-- Server version	8.0.42-0ubuntu0.22.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `device_registry`
--

DROP TABLE IF EXISTS `device_registry`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `device_registry` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `ip` varchar(100) NOT NULL,
  `port` int NOT NULL,
  `registered_at` timestamp NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=303 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `device_registry`
--

LOCK TABLES `device_registry` WRITE;
/*!40000 ALTER TABLE `device_registry` DISABLE KEYS */;
INSERT INTO `device_registry` VALUES (298,'actuator_light','fd00:0:0:0:206:6:6:6',5683,'2025-07-13 17:04:03'),(299,'sensor_temp','fd00:0:0:0:202:2:2:2',5683,'2025-07-13 17:04:05'),(300,'sensor_humidity','fd00:0:0:0:204:4:4:4',5683,'2025-07-13 17:04:05'),(301,'actuator_temp','fd00:0:0:0:205:5:5:5',5683,'2025-07-13 17:04:05'),(302,'sensor_light','fd00:0:0:0:203:3:3:3',5683,'2025-07-13 17:04:06');
/*!40000 ALTER TABLE `device_registry` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `humidity_data`
--

DROP TABLE IF EXISTS `humidity_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `humidity_data` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `value` float NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=479 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `humidity_data`
--

LOCK TABLES `humidity_data` WRITE;
/*!40000 ALTER TABLE `humidity_data` DISABLE KEYS */;
INSERT INTO `humidity_data` VALUES (435,'sensor_humidity',37.816,'2025-07-13 17:04:05'),(436,'sensor_humidity',35.444,'2025-07-13 17:04:13'),(437,'sensor_humidity',33.072,'2025-07-13 17:04:18'),(438,'sensor_humidity',30.7,'2025-07-13 17:04:23'),(439,'sensor_humidity',28.328,'2025-07-13 17:04:28'),(440,'sensor_humidity',25.956,'2025-07-13 17:04:33'),(441,'sensor_humidity',23.584,'2025-07-13 17:04:38'),(442,'sensor_humidity',21.212,'2025-07-13 17:04:43'),(443,'sensor_humidity',18.84,'2025-07-13 17:04:48'),(444,'sensor_humidity',16.468,'2025-07-13 17:04:53'),(445,'sensor_humidity',14.096,'2025-07-13 17:04:58'),(446,'sensor_humidity',11.724,'2025-07-13 17:05:03'),(447,'sensor_humidity',10,'2025-07-13 17:05:08'),(448,'sensor_humidity',12.372,'2025-07-13 17:05:13'),(449,'sensor_humidity',14.744,'2025-07-13 17:05:18'),(450,'sensor_humidity',17.116,'2025-07-13 17:05:23'),(451,'sensor_humidity',19.488,'2025-07-13 17:05:28'),(452,'sensor_humidity',21.86,'2025-07-13 17:05:33'),(453,'sensor_humidity',24.232,'2025-07-13 17:05:38'),(454,'sensor_humidity',26.604,'2025-07-13 17:05:43'),(455,'sensor_humidity',28.976,'2025-07-13 17:05:48'),(456,'sensor_humidity',31.348,'2025-07-13 17:05:53'),(457,'sensor_humidity',33.72,'2025-07-13 17:05:58'),(458,'sensor_humidity',36.092,'2025-07-13 17:06:03'),(459,'sensor_humidity',38.464,'2025-07-13 17:06:08'),(460,'sensor_humidity',40.836,'2025-07-13 17:06:13'),(461,'sensor_humidity',43.208,'2025-07-13 17:06:18'),(462,'sensor_humidity',45.58,'2025-07-13 17:06:23'),(463,'sensor_humidity',47.952,'2025-07-13 17:06:28'),(464,'sensor_humidity',50.324,'2025-07-13 17:06:33'),(465,'sensor_humidity',52.696,'2025-07-13 17:06:38'),(466,'sensor_humidity',55.068,'2025-07-13 17:06:43'),(467,'sensor_humidity',57.44,'2025-07-13 17:06:48'),(468,'sensor_humidity',59.812,'2025-07-13 17:06:53'),(469,'sensor_humidity',62.184,'2025-07-13 17:06:58'),(470,'sensor_humidity',64.556,'2025-07-13 17:07:03'),(471,'sensor_humidity',66.928,'2025-07-13 17:07:08'),(472,'sensor_humidity',69.3,'2025-07-13 17:07:13'),(473,'sensor_humidity',71.672,'2025-07-13 17:07:18'),(474,'sensor_humidity',74.044,'2025-07-13 17:07:23'),(475,'sensor_humidity',76.416,'2025-07-13 17:07:28'),(476,'sensor_humidity',78.788,'2025-07-13 17:07:33'),(477,'sensor_humidity',81.16,'2025-07-13 17:07:38'),(478,'sensor_humidity',83.532,'2025-07-13 17:07:43');
/*!40000 ALTER TABLE `humidity_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `light_actuator`
--

DROP TABLE IF EXISTS `light_actuator`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `light_actuator` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `value` int NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `light_actuator`
--

LOCK TABLES `light_actuator` WRITE;
/*!40000 ALTER TABLE `light_actuator` DISABLE KEYS */;
INSERT INTO `light_actuator` VALUES (15,'actuator_light',0,'2025-07-13 17:04:03'),(16,'actuator_light',1,'2025-07-13 17:05:05'),(17,'actuator_light',1,'2025-07-13 17:06:07'),(18,'actuator_light',1,'2025-07-13 17:07:09');
/*!40000 ALTER TABLE `light_actuator` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `light_data`
--

DROP TABLE IF EXISTS `light_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `light_data` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `value` float NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=480 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `light_data`
--

LOCK TABLES `light_data` WRITE;
/*!40000 ALTER TABLE `light_data` DISABLE KEYS */;
INSERT INTO `light_data` VALUES (436,'sensor_light',268.443,'2025-07-13 17:04:06'),(437,'sensor_light',245.052,'2025-07-13 17:04:13'),(438,'sensor_light',221.661,'2025-07-13 17:04:18'),(439,'sensor_light',198.27,'2025-07-13 17:04:23'),(440,'sensor_light',174.879,'2025-07-13 17:04:28'),(441,'sensor_light',151.488,'2025-07-13 17:04:33'),(442,'sensor_light',128.097,'2025-07-13 17:04:38'),(443,'sensor_light',104.706,'2025-07-13 17:04:43'),(444,'sensor_light',100,'2025-07-13 17:04:48'),(445,'sensor_light',123.391,'2025-07-13 17:04:53'),(446,'sensor_light',146.782,'2025-07-13 17:04:58'),(447,'sensor_light',170.173,'2025-07-13 17:05:03'),(448,'sensor_light',193.564,'2025-07-13 17:05:08'),(449,'sensor_light',216.955,'2025-07-13 17:05:13'),(450,'sensor_light',240.346,'2025-07-13 17:05:18'),(451,'sensor_light',263.737,'2025-07-13 17:05:23'),(452,'sensor_light',287.128,'2025-07-13 17:05:28'),(453,'sensor_light',310.519,'2025-07-13 17:05:33'),(454,'sensor_light',333.91,'2025-07-13 17:05:38'),(455,'sensor_light',357.301,'2025-07-13 17:05:43'),(456,'sensor_light',380.692,'2025-07-13 17:05:48'),(457,'sensor_light',404.083,'2025-07-13 17:05:53'),(458,'sensor_light',427.474,'2025-07-13 17:05:58'),(459,'sensor_light',450.865,'2025-07-13 17:06:03'),(460,'sensor_light',474.256,'2025-07-13 17:06:08'),(461,'sensor_light',497.647,'2025-07-13 17:06:13'),(462,'sensor_light',500,'2025-07-13 17:06:18'),(463,'sensor_light',476.609,'2025-07-13 17:06:23'),(464,'sensor_light',453.218,'2025-07-13 17:06:28'),(465,'sensor_light',429.827,'2025-07-13 17:06:33'),(466,'sensor_light',406.436,'2025-07-13 17:06:38'),(467,'sensor_light',383.045,'2025-07-13 17:06:43'),(468,'sensor_light',359.654,'2025-07-13 17:06:48'),(469,'sensor_light',336.263,'2025-07-13 17:06:53'),(470,'sensor_light',312.872,'2025-07-13 17:06:58'),(471,'sensor_light',289.481,'2025-07-13 17:07:03'),(472,'sensor_light',266.09,'2025-07-13 17:07:08'),(473,'sensor_light',242.699,'2025-07-13 17:07:13'),(474,'sensor_light',219.308,'2025-07-13 17:07:18'),(475,'sensor_light',195.917,'2025-07-13 17:07:23'),(476,'sensor_light',172.526,'2025-07-13 17:07:28'),(477,'sensor_light',149.135,'2025-07-13 17:07:33'),(478,'sensor_light',125.744,'2025-07-13 17:07:38'),(479,'sensor_light',102.353,'2025-07-13 17:07:43');
/*!40000 ALTER TABLE `light_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `temperature_actuator`
--

DROP TABLE IF EXISTS `temperature_actuator`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `temperature_actuator` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `value` int NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `temperature_actuator`
--

LOCK TABLES `temperature_actuator` WRITE;
/*!40000 ALTER TABLE `temperature_actuator` DISABLE KEYS */;
INSERT INTO `temperature_actuator` VALUES (15,'actuator_temp',0,'2025-07-13 17:04:06'),(16,'actuator_temp',1,'2025-07-13 17:05:08'),(17,'actuator_temp',1,'2025-07-13 17:06:10'),(18,'actuator_temp',1,'2025-07-13 17:07:12');
/*!40000 ALTER TABLE `temperature_actuator` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `temperature_data`
--

DROP TABLE IF EXISTS `temperature_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `temperature_data` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `value` float NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=479 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `temperature_data`
--

LOCK TABLES `temperature_data` WRITE;
/*!40000 ALTER TABLE `temperature_data` DISABLE KEYS */;
INSERT INTO `temperature_data` VALUES (435,'sensor_temp',14.915,'2025-07-13 17:04:08'),(436,'sensor_temp',15.898,'2025-07-13 17:04:13'),(437,'sensor_temp',16.881,'2025-07-13 17:04:18'),(438,'sensor_temp',17.864,'2025-07-13 17:04:23'),(439,'sensor_temp',18.847,'2025-07-13 17:04:28'),(440,'sensor_temp',19.83,'2025-07-13 17:04:33'),(441,'sensor_temp',20.813,'2025-07-13 17:04:38'),(442,'sensor_temp',21.796,'2025-07-13 17:04:43'),(443,'sensor_temp',22.779,'2025-07-13 17:04:48'),(444,'sensor_temp',23.762,'2025-07-13 17:04:53'),(445,'sensor_temp',24.745,'2025-07-13 17:04:58'),(446,'sensor_temp',25.728,'2025-07-13 17:05:03'),(447,'sensor_temp',26.711,'2025-07-13 17:05:08'),(448,'sensor_temp',27.694,'2025-07-13 17:05:13'),(449,'sensor_temp',28.677,'2025-07-13 17:05:18'),(450,'sensor_temp',29.66,'2025-07-13 17:05:23'),(451,'sensor_temp',30.643,'2025-07-13 17:05:28'),(452,'sensor_temp',31.626,'2025-07-13 17:05:33'),(453,'sensor_temp',32.609,'2025-07-13 17:05:38'),(454,'sensor_temp',33.592,'2025-07-13 17:05:43'),(455,'sensor_temp',34.575,'2025-07-13 17:05:48'),(456,'sensor_temp',35.558,'2025-07-13 17:05:53'),(457,'sensor_temp',36.541,'2025-07-13 17:05:58'),(458,'sensor_temp',37.524,'2025-07-13 17:06:03'),(459,'sensor_temp',38.507,'2025-07-13 17:06:08'),(460,'sensor_temp',39.49,'2025-07-13 17:06:13'),(461,'sensor_temp',40,'2025-07-13 17:06:18'),(462,'sensor_temp',39.017,'2025-07-13 17:06:23'),(463,'sensor_temp',38.034,'2025-07-13 17:06:28'),(464,'sensor_temp',37.051,'2025-07-13 17:06:33'),(465,'sensor_temp',36.068,'2025-07-13 17:06:38'),(466,'sensor_temp',35.085,'2025-07-13 17:06:43'),(467,'sensor_temp',34.102,'2025-07-13 17:06:48'),(468,'sensor_temp',33.119,'2025-07-13 17:06:53'),(469,'sensor_temp',32.136,'2025-07-13 17:06:58'),(470,'sensor_temp',31.153,'2025-07-13 17:07:03'),(471,'sensor_temp',30.17,'2025-07-13 17:07:08'),(472,'sensor_temp',29.187,'2025-07-13 17:07:13'),(473,'sensor_temp',28.204,'2025-07-13 17:07:18'),(474,'sensor_temp',27.221,'2025-07-13 17:07:23'),(475,'sensor_temp',26.238,'2025-07-13 17:07:28'),(476,'sensor_temp',25.255,'2025-07-13 17:07:33'),(477,'sensor_temp',24.272,'2025-07-13 17:07:38'),(478,'sensor_temp',23.289,'2025-07-13 17:07:43');
/*!40000 ALTER TABLE `temperature_data` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-07-13 19:11:23
