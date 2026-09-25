package tamaf;

public class AgentID {
    private final String name;
    private Address address;

    public AgentID(String name) {
        this.name = name;
    }

    public AgentID(String name, Address address) {
        this.name = name;
        this.address = address;
    }

    public String getName() { return name; }
    public Address getAddress() { return address; }

    public void setPort(int port) {
        if (this.address != null) {
            this.address.setPort(port);
        }
    }

    public String getFullID() {
        if (address == null) return name + "@None:None";
        return name + "@" + address.getIp() + ":" + (address.getPort() != null ? address.getPort() : "None");
    }

    public static AgentID fromString(String aidString) {
        String[] parts = aidString.split("@", 2);
        String name = parts[0];
        String networkPart = parts[1];
        
        if (name.equals("None")) name = null;
        
        int lastColon = networkPart.lastIndexOf(':');
        String ip = networkPart.substring(0, lastColon);
        String portStr = networkPart.substring(lastColon + 1);
        
        if (ip.equals("None")) {
            return new AgentID(name, null);
        }
        
        Integer port = portStr.equals("None") ? null : Integer.valueOf(portStr);
        return new AgentID(name, new Address(ip, port));
    }

    public static boolean matches(AgentID template, AgentID target) {
        if (target == null) return false;
        
        // If template has a name, it must match.
        // In this framework, name@address is the full ID, but name should be unique enough for most templates.
        if (template.getName() != null) {
            if (template.getName().equals(target.getName())) {
                // Name matches. If address is also provided in template, we can check it,
                // but for cross-platform/multi-IP stability, we should be lenient if only name was intended.
                // If the template has NO address, we consider it a name-only match.
                if (template.getAddress() == null) {
                    return true;
                }
                
                // If template HAS address, we check it.
                if (target.getAddress() == null) return false;
                
                if (template.getAddress().getIp() != null && !template.getAddress().getIp().equals(target.getAddress().getIp())) {
                    return false;
                }
                
                if (template.getAddress().getPort() != null && !template.getAddress().getPort().equals(target.getAddress().getPort())) {
                    return false;
                }
                
                return true;
            } else {
                return false;
            }
        }
        
        // If name is null in template, we must match by address
        if (template.getAddress() != null) {
            if (target.getAddress() == null) return false;
            
            if (template.getAddress().getIp() != null && !template.getAddress().getIp().equals(target.getAddress().getIp())) {
                return false;
            }
            
            if (template.getAddress().getPort() != null && !template.getAddress().getPort().equals(target.getAddress().getPort())) {
                return false;
            }
            return true;
        }

        // Empty template matches everything
        return true;
    }
}
