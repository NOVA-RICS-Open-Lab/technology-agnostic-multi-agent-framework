package tamaf.defines;


public class Defines {
    private static String getEnv(String key, String defaultValue) {
        String val = System.getProperty(key, System.getenv(key));
        return val != null ? val : defaultValue;
    }

    private static int getEnvInt(String key, int defaultValue) {
        String val = System.getProperty(key, System.getenv(key));
        try {
            return val != null ? Integer.parseInt(val) : defaultValue;
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }

    private static double getEnvDouble(String key, double defaultValue) {
        String val = System.getProperty(key, System.getenv(key));
        try {
            return val != null ? Double.parseDouble(val) : defaultValue;
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }

    public static final double DEFAULT_REGISTRATION_PORT_RETRY_TIME = getEnvDouble("DEFAULT_REGISTRATION_PORT_RETRY_TIME", 0.5);
    public static final int DEFAULT_MESSAGE_TIMEOUT_TIME = getEnvInt("DEFAULT_MESSAGE_TIMEOUT_TIME", 5);
    public static final int DEFAULT_TCP_BACKLOG = getEnvInt("DEFAULT_TCP_BACKLOG", 10);
    public static final double SERVER_STARTING_TIME = getEnvDouble("SERVER_STARTING_TIME", 0.5);

    public static final int DEFAULT_EMA_REGISTER_PORT = getEnvInt("DEFAULT_EMA_REGISTER_PORT", 4001);
    public static final int DEFAULT_EMA_HEARTBEAT_INTERVAL = getEnvInt("DEFAULT_EMA_HEARTBEAT_INTERVAL", 5000);
    public static final int DEFAULT_EMA_MESSAGE_RETRYS = getEnvInt("DEFAULT_EMA_MESSAGE_RETRYS", 3);
    public static final double DEFAULT_EMA_MESSAGE_RETRY_INTERVAL = getEnvDouble("DEFAULT_EMA_MESSAGE_RETRY_INTERVAL", 0.5);
    public static final int DEFAULT_EMA_HEARTBEAT_FAILURE_THRESHOLD = getEnvInt("DEFAULT_EMA_HEARTBEAT_FAILURE_THRESHOLD", 5);
    public static final int DEFAULT_AGENT_HEARTBEAT_FAILURE_THRESHOLD = getEnvInt("DEFAULT_AGENT_HEARTBEAT_FAILURE_THRESHOLD", 5);
    public static final double DEFAULT_EMA_EXTERNAL_HEARTBEAT_TIMEOUT = getEnvDouble("DEFAULT_EMA_EXTERNAL_HEARTBEAT_TIMEOUT", 0.5);
    
    public static final int DEFAULT_CONTRACTNETINITIATOR_RECEIVE_CFPS_TIMEOUT = getEnvInt("DEFAULT_CONTRACTNETINITIATOR_RECEIVE_CFPS_TIMEOUT", 5);

    public static final String DEFAULT_MULTICAST_IP = getEnv("DEFAULT_MULTICAST_IP", "224.1.1.1");
    public static final int DEFAULT_MULTICAST_PORT = getEnvInt("DEFAULT_MULTICAST_PORT", 4002);

    public static final String LOCALHOST = "127.0.0.1";
    public static final String ALL_INTERFACES = "0.0.0.0";
    public static final int DEFAULT_EMA_PORT = 4000;
    public static final int DEFAULT_UDP_PACKETSIZE = 65535;
    public static final double DEFAULT_KERNEL_STABILIZATION_TIME = 0.1;
    public static final double DEFAULT_LOOP_STABILIZATION_TIME = 0.1;
    public static final String DEFAULT_EMA_NAME = "EMA";
    public static final int DEFAULT_THREAD_JOIN_TIMEOUT_TIME = 2;
    public static final int DEFAULT_AGENT_ALREADY_EXISTS = -1;
    public static final int DEFAULT_EMA_HAS_NO_MORE_PORTS = -2;
    public static final int DEFAULT_OUTSIDE_AGENT = -3;

    // Ontologies and Actions
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT = "am-agent-address";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT = "am-inform";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT = "am-agent-description";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME = "tamaf-agent-management";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION = "register";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_RESULT_ACTION = "register-result";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION = "deregister";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_RESULT_ACTION = "deregister-result";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION = "modify";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_RESULT_ACTION = "modify-result";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION = "local-search";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION = "external-search";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION = "global-search";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION = "search-result";
    public static final String DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION = "keepalive";
    public static final String DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT = "ema-environment-lastupdatetime";
    public static final String DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_NAME = "tamaf-environment-management";
    public static final String DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_SEARCH_ACTION = "search";
    public static final String DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION = "keepalive";


}
