package org.cts.utilities;

public class Constants {

    //Global Deployment
    public static final String DF_SERVICE_TYPE_SKILL = "SKILL_TYPE";
    //Local Deployment
    public static final String DF_SERVICE_TYPE_TRANSPORT = "TRANSPORT_TYPE";
    public static final String DF_SERVICE_TYPE_RESOURCE = "RESOURCE_TYPE";;
    public static final String DF_SERVICE_NAME_TRANSPORT = "TRANSPORT_NAME";

    //Ontologies
    public static final String ONTOLOGY_NEGOTIATE_NEXT_RESOURCE = "CNET_NEXT_RESOURCE";
    public static final String ONTOLOGY_REQUEST_TRANSPORT = "REQ_TRANSPORT";
    public static final String ONTOLOGY_REQUEST_EXECUTE_SKILL = "REQ_EXECUTE_SKILL";
    public static final String ONTOLOGY_REQUEST_UNPLUG_RESOURCE = "REQ_UNPLUG_RESOURCE";
    public static final String ONTOLOGY_REQUEST_UPDATE_CONFIG = "REQ_UPDATE_CONFIG";
    
    public static String controllerIP = "";

    public enum ResourceType {
        Robot,Station,Human
    }

    public enum Locations {
        A,B,C,D,E,F
    }

    public String[][] skills = {
        {"Drill", Constants.DF_SERVICE_TYPE_RESOURCE, "http://" + controllerIP + "/estacao?skill=0"},
        {"Screw", Constants.DF_SERVICE_TYPE_RESOURCE, "http://" + controllerIP + "/estacao?skill=1"},
        {DF_SERVICE_NAME_TRANSPORT, Constants.DF_SERVICE_TYPE_TRANSPORT, "http://" + controllerIP + "/passadeiras?skill=AB"},
        {"GoToAB", Constants.DF_SERVICE_TYPE_TRANSPORT, "http://" + controllerIP + "/passadeiras?skill=AB"},
        {"GoToBC", Constants.DF_SERVICE_TYPE_TRANSPORT, "http://" + controllerIP + "/passadeiras?skill=BC"},
        {"GoToCD", Constants.DF_SERVICE_TYPE_TRANSPORT, "http://" + controllerIP + "/passadeiras?skill=CD"},
        {"GoToDE", Constants.DF_SERVICE_TYPE_TRANSPORT, "http://" + controllerIP + "/passadeiras?skill=DE"},
        {"GoToEF", Constants.DF_SERVICE_TYPE_TRANSPORT, "http://" + controllerIP + "/passadeiras?skill=EF"},
        {"GoToFA", Constants.DF_SERVICE_TYPE_TRANSPORT, "http://" + controllerIP + "/passadeiras?skill=FA"},
    };

    public static String getHostTag() {
        java.io.File file = new java.io.File("/proc/device-tree/model");
        if (file.exists()) {
            try {
                byte[] bytes = java.nio.file.Files.readAllBytes(file.toPath());
                String content = new String(bytes).toLowerCase();
                if (content.contains("raspberry pi")) {
                    return "RPi";
                }
            } catch (Exception e) {}
        }
        return "PC";
    }

    public static String formatAgentName(String name) {
        String tag = getHostTag();
        if (name != null && !name.endsWith("_" + tag)) {
            return name + "_" + tag;
        }
        return name;
    }
}
