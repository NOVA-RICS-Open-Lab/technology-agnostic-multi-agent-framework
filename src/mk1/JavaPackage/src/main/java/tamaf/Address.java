package tamaf;

public class Address {
    private final String ip;
    private Integer port;

    public Address(String ip) {
        this.ip = ip;
    }

    public Address(String ip, Integer port) {
        this.ip = ip;
        this.port = port;
    }

    public String getIp() { return ip; }
    public Integer getPort() { return port; }
    public void setPort(Integer port) { this.port = port; }

    public String getTuple() { return ip + ":" + (port != null ? port : "None"); }
    public String getHttpLink() { return "http://" + ip + ":" + (port != null ? port : "None"); }
}
