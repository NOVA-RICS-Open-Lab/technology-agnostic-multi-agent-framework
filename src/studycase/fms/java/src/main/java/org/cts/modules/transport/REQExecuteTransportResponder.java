package org.cts.modules.transport;

import org.cts.modules.resource.libraries.kitt_library;
import org.cts.modules.skilled.libraries.executionLibrary;
import org.cts.utilities.Constants;

import org.json.JSONObject;
import tamaf.*;

import java.io.IOException;

public class REQExecuteTransportResponder extends FIPARequestResponder {

    private Thread executionThread;

    public REQExecuteTransportResponder(Agent a, ACLMessageTemplate mt) {
        super(a, mt);
    }

    @Override
    public ACLMessage handleRequest(ACLMessage request) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received REQUEST execute transport");
        ACLMessage reply = request.createReply();

        String jsonContent = request.getContent().toString();

        // 2. Parse it into a JSONObject
        JSONObject jsonObject = new JSONObject(jsonContent);

        // 3. Extract the two strings using their keys
        String myLocation = jsonObject.getString("a");
        String nextLocation = jsonObject.getString("b");

        if(validLocation(nextLocation)){
            System.out.println("DEBUG: " + this.agent.getName() + " Valid location - Starting transport");
            reply.setPerformative(Performative.AGREE);
            ((TransportAgent) this.agent).currentProductLocation = myLocation;
        }
        else{
            System.out.println("DEBUG: " + this.agent.getName() + " Invalid location - Transport refused");
            reply.setPerformative(Performative.REFUSE);
        }
        return reply;
    }

    private boolean validLocation(String location) {
        if(location.matches(Constants.Locations.A.name()) ||
                location.matches(Constants.Locations.B.name()) ||
                location.matches(Constants.Locations.C.name()) ||
                location.matches(Constants.Locations.D.name()) ||
                location.matches(Constants.Locations.E.name()) ||
                location.matches(Constants.Locations.F.name())
        )
            return true;
        else
            return false;
    }

    @Override
    public ACLMessage prepareResultNotification(ACLMessage request, ACLMessage response){
        System.out.println("DEBUG: " + this.agent.getName() + " Executing transport");

        String jsonContent = request.getContent().toString();

        // 2. Parse it into a JSONObject
        JSONObject jsonObject = new JSONObject(jsonContent);

        String nextLocation = jsonObject.getString("b");
        //EXECUTE TRANSPORT
        try {
            if (!executeTransport(nextLocation)) {
                return null;
            }
        } catch (IOException e) {
            System.out.println("\nADAWDADWDAW: " + e + "\n");
            throw new RuntimeException(e);
        }

        ACLMessage reply = request.createReply();
        reply.setPerformative(Performative.INFORM);
        reply.setContent(((TransportAgent) this.agent).currentProductLocation);
        System.out.println("DEBUG: " + this.agent.getName() + " Transport finished");
        return reply;
    }

    private boolean executeTransport(String futureLocation) throws IOException {
        Constants.Locations[] myLocations = Constants.Locations.values();
        String currentLocation = ((TransportAgent) this.agent).currentProductLocation;

        // 1. Verifica se já está no destino
        if (currentLocation.equals(futureLocation)) {
            System.out.println("Already in position: " + futureLocation);
            return true;
        }

        // 2. Encontra o índice da localização atual
        int currentIndex = 0;
        for (int i = 0; i < myLocations.length; i++) {
            if (myLocations[i].name().equals(currentLocation)) {
                currentIndex = i;
                break;
            }
        }

        // 3. Calcula qual é o próximo passo (com lógica circular)
        int nextIndex = currentIndex + 1;
        // Em vez de usar o número '5' fixo, usamos o tamanho do array para saber se chegámos ao fim
        if (nextIndex >= myLocations.length) {
            nextIndex = 0; // Volta para o início (wrap-around)
        }

        String nextLocation = myLocations[nextIndex].name();
        String currLocationName = myLocations[currentIndex].name();

        // 4. Executa o movimento para a PRÓXIMA estação apenas
        System.out.println("DEBUG: Call transport from " + currLocationName + " to " + nextLocation);

        kitt_library.executeTransporte(currLocationName + nextLocation);
        ((TransportAgent) this.agent).currentProductLocation = nextLocation;

        // 5. Verifica se este único passo o levou ao destino final
        if (nextLocation.equals(futureLocation)) {
            System.out.println("Arrived: " + futureLocation);
            return true;
        } else {
            System.out.println("Step completed. Currently at: " + nextLocation + ". Target is: " + futureLocation);
            return false;
        }
    }
}
