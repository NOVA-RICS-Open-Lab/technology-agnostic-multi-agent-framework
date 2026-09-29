package tamaf;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ServiceDescription {
    private String name;
    private String type;
    private String ownership;
    private List<String> protocols = new ArrayList<>();
    private List<String> ontologies = new ArrayList<>();
    private Map<String, Object> properties = new HashMap<>();

    public ServiceDescription(String name) {
        this.name = name;
    }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }
    public String getType() { return type; }
    public void setType(String type) { this.type = type; }
    public String getOwnership() { return ownership; }
    public void setOwnership(String ownership) { this.ownership = ownership; }
    public List<String> getProtocols() { return protocols; }
    public void setProtocols(List<String> protocols) { this.protocols = protocols; }
    public List<String> getOntologies() { return ontologies; }
    public void setOntologies(List<String> ontologies) { this.ontologies = ontologies; }
    public Map<String, Object> getProperties() { return properties; }
    public void setProperties(Map<String, Object> properties) { this.properties = properties; }

    public boolean matches(ServiceDescription template) {
        if (template.getName() != null && !template.getName().equals(this.name)) return false;
        if (template.getType() != null && !template.getType().equals(this.type)) return false;
        if (template.getOwnership() != null && !template.getOwnership().equals(this.ownership)) return false;
        
        if (template.getProtocols() != null && !this.protocols.containsAll(template.getProtocols())) return false;
        if (template.getOntologies() != null && !this.ontologies.containsAll(template.getOntologies())) return false;
        
        if (template.getProperties() != null) {
            for (Map.Entry<String, Object> entry : template.getProperties().entrySet()) {
                if (!entry.getValue().equals(this.properties.get(entry.getKey()))) return false;
            }
        }
        return true;
    }

    public Map<String, Object> toMap() {
        Map<String, Object> map = new HashMap<>();
        map.put("name", name);
        map.put("type", type);
        map.put("ownership", ownership);
        map.put("protocols", protocols);
        map.put("ontologies", ontologies);
        map.put("properties", properties);
        return map;
    }

    @SuppressWarnings("unchecked")
    public static ServiceDescription fromMap(Map<String, Object> map) {
        if (map == null) return null;
        ServiceDescription sd = new ServiceDescription((String) map.get("name"));
        sd.setType((String) map.get("type"));
        sd.setOwnership((String) map.get("ownership"));
        sd.setProtocols((List<String>) map.get("protocols"));
        sd.setOntologies((List<String>) map.get("ontologies"));
        sd.setProperties((Map<String, Object>) map.get("properties"));
        return sd;
    }
}
