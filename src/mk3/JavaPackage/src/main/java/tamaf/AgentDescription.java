package tamaf;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class AgentDescription {
    private AgentID agentid;
    private final List<ServiceDescription> services = new ArrayList<>();

    public AgentDescription() {}
    public AgentDescription(AgentID agentid) { this.agentid = agentid; }

    public AgentID getAgentid() { return agentid; }
    public void setAgentid(AgentID agentid) { this.agentid = agentid; }
    public List<ServiceDescription> getServices() { return services; }

    public void addService(ServiceDescription service) { this.services.add(service); }

    public boolean matches(AgentDescription template) {
        if (template.getAgentid() != null) {
            if (!AgentID.matches(template.getAgentid(), this.agentid)){
                return false;
            }
        }

        for (ServiceDescription templateService : template.getServices()) {
            boolean found = false;
            for (ServiceDescription service : this.services) {
                if (service.matches(templateService)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    public Map<String, Object> toMap() {
        Map<String, Object> map = new HashMap<>();
        if (agentid != null) map.put("agentid", agentid.getFullID());
        List<Map<String, Object>> serviceMaps = new ArrayList<>();
        for (ServiceDescription sd : services) serviceMaps.add(sd.toMap());
        map.put("services", serviceMaps);
        return map;
    }

    @SuppressWarnings("unchecked")
    public static AgentDescription fromMap(Map<String, Object> map) {
        if (map == null) return null;
        String aidText = (String) map.get("agentid");
        AgentDescription ad = new AgentDescription(aidText != null ? AgentID.fromString(aidText) : null);
        List<Map<String, Object>> serviceMaps = (List<Map<String, Object>>) map.get("services");
        if (serviceMaps != null) {
            for (Map<String, Object> sm : serviceMaps) {
                ad.addService(ServiceDescription.fromMap(sm));
            }
        }
        return ad;
    }
}
