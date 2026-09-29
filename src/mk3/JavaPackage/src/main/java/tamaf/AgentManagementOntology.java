package tamaf;

import tamaf.defines.Defines;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class AgentManagementOntology {
    public static final String NAME = Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME;
    
    public static final List<String> ACTIONS = Arrays.asList(
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_RESULT_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_RESULT_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_RESULT_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION
    );

    public static final List<String> CONCEPTS = Arrays.asList(
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT,
        Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT
    );

    public static boolean validate(Map<String, Object> content) {
        String action = (String) content.get("action");
        String concept = (String) content.get("concept");
        if (!ACTIONS.contains(action)) return false;
        if (!CONCEPTS.contains(concept)) return false;
        return true;
    }

    public static String getAction(Map<String, Object> content) {
        if (!validate(content)) return null;
        return (String) content.get("action");
    }

    public static String getConcept(Map<String, Object> content) {
        if (!validate(content)) return null;
        return (String) content.get("concept");
    }

    public static Object getAttributes(Map<String, Object> content) {
        if (!validate(content)) return null;
        return content.get("attributes");
    }

    @SuppressWarnings("unchecked")
    public static List<AgentDescription> getAgentDescriptionsFromAttributes(Map<String, Object> content) {
        if (!validate(content)) return null;
        String concept = (String) content.get("concept");
        if (Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT.equals(concept)) {
            List<Map<String, Object>> attrs = (List<Map<String, Object>>) content.get("attributes");
            List<AgentDescription> result = new ArrayList<>();
            if (attrs != null) {
                for (Map<String, Object> attr : attrs) {
                    result.add(AgentDescription.fromMap(attr));
                }
            }
            return result;
        }
        return null;
    }

    public static Address getAgentAddressFromAttributes(Map<String, Object> content) {
        if (!validate(content)) return null;
        String concept = (String) content.get("concept");
        if (Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT.equals(concept)) {
            return Address.fromString((String) content.get("attributes"));
        }
        return null;
    }

    public static Map<String, Object> createAgentDescriptionMessageContent(String action, List<AgentDescription> ads) {
        Map<String, Object> content = new HashMap<>();
        content.put("action", action);
        content.put("concept", Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT);
        List<Map<String, Object>> adMaps = new ArrayList<>();
        for (AgentDescription ad : ads) adMaps.add(ad.toMap());
        content.put("attributes", adMaps);
        return content;
    }

    public static Map<String, Object> createAgentAddressMessageContent(String action, Address agentAddress) {
        Map<String, Object> content = new HashMap<>();
        content.put("action", action);
        content.put("concept", Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT);
        content.put("attributes", agentAddress.getString());
        return content;
    }
    
    public static Map<String, Object> createResultMessageContent(String action, String concept, int inform) {
        Map<String, Object> content = new HashMap<>();
        content.put("action", action);
        content.put("concept", concept);
        content.put("attributes", inform);
        return content;
    }
}
