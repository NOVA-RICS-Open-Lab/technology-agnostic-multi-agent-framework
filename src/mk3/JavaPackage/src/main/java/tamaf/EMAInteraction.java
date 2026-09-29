package tamaf;

import tamaf.defines.Defines;
import tamaf.defines.EMAFailureException;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

public class EMAInteraction {
    private final Behavior behavior;
    private boolean messageSent = false;
    private final String conversationID;

    public EMAInteraction(Behavior behavior) {
        this.behavior = behavior;
        this.conversationID = "ema-" + UUID.randomUUID().toString();
    }

    protected void sendToEMA(ACLMessage msg) {
        for (int i = 0; i < Defines.DEFAULT_EMA_MESSAGE_RETRYS; i++) {
            boolean sent = behavior.send(msg);
            if (sent) return;
            try {
                Thread.sleep((long) (Defines.DEFAULT_EMA_MESSAGE_RETRY_INTERVAL * 1000));
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new EMAFailureException("Interrupted while waiting to retry EMA message.");
            }
        }
        throw new EMAFailureException();
    }

    public void keepAliveAgent() {
        ACLMessage aclMessage = new ACLMessage(Performative.KEEPALIVE);
        aclMessage.setSender(this.behavior.getAgent().getAgentDescription().getAgentid());
        aclMessage.setReceiver(Collections.singletonList(new AgentID(Defines.DEFAULT_EMA_NAME, this.behavior.getAgent().mts.getEmaAddress())));
        aclMessage.setContent(AgentManagementOntology.createAgentDescriptionMessageContent(
                Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION, 
                Collections.singletonList(this.behavior.getAgent().getAgentDescription())
        ));
        aclMessage.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        
        this.sendToEMA(aclMessage);
    }

    public ACLMessage registerAgent() {
        if (!messageSent) {
            ACLMessage msg = new ACLMessage(Performative.REQUEST);
            msg.setSender(behavior.getAgent().getAgentDescription().getAgentid());
            msg.setReceiver(Collections.singletonList(new AgentID(Defines.DEFAULT_EMA_NAME, behavior.getAgent().mts.getEmaAddress())));
            msg.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            msg.setContent(AgentManagementOntology.createAgentDescriptionMessageContent(
                    Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION,
                    Collections.singletonList(behavior.getAgent().getAgentDescription())
            ));
            msg.setConversationId(conversationID);

            sendToEMA(msg);
            messageSent = true;
        }

        if (messageSent) {
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setSender(new AgentID(Defines.DEFAULT_EMA_NAME));
            template.setPerformatives(Collections.singleton(Performative.INFORM));
            template.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            template.setConversationId(conversationID);

            behavior.getAgent().logDebug(2, "Waiting for response from EMA...");
            ACLMessage response = behavior.receive(template);
            behavior.getAgent().logDebug(2, "Response received from EMA: " + (response != null ? response.getContent() : "null"));
            if (response != null) {
                messageSent = false;
                return response;
            }
        }
        return null;
    }

    public ACLMessage deRegisterAgent() {
        if (!messageSent) {
            ACLMessage msg = new ACLMessage(Performative.REQUEST);
            msg.setSender(behavior.getAgent().getAgentDescription().getAgentid());
            msg.setReceiver(Collections.singletonList(new AgentID(Defines.DEFAULT_EMA_NAME, behavior.getAgent().mts.getEmaAddress())));
            msg.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            msg.setContent(AgentManagementOntology.createAgentDescriptionMessageContent(
                    Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION,
                    Collections.singletonList(behavior.getAgent().getAgentDescription())
            ));
            msg.setConversationId(conversationID);

            sendToEMA(msg);
            messageSent = true;
        }

        if (messageSent) {
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setSender(new AgentID(Defines.DEFAULT_EMA_NAME));
            template.setPerformatives(Collections.singleton(Performative.INFORM));
            template.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            template.setConversationId(conversationID);

            ACLMessage response = behavior.receive(template);
            if (response != null) {
                messageSent = false;
                return response;
            }
        }
        return null;
    }

    public ACLMessage modifyAgent() {
        if (!messageSent) {
            ACLMessage msg = new ACLMessage(Performative.REQUEST);
            msg.setSender(behavior.getAgent().getAgentDescription().getAgentid());
            msg.setReceiver(Collections.singletonList(new AgentID(Defines.DEFAULT_EMA_NAME, behavior.getAgent().mts.getEmaAddress())));
            msg.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            msg.setContent(AgentManagementOntology.createAgentDescriptionMessageContent(
                    Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION,
                    Collections.singletonList(behavior.getAgent().getAgentDescription())
            ));
            msg.setConversationId(conversationID);

            sendToEMA(msg);
            messageSent = true;
        }

        if (messageSent) {
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setSender(new AgentID(Defines.DEFAULT_EMA_NAME));
            template.setPerformatives(Collections.singleton(Performative.INFORM));
            template.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            template.setConversationId(conversationID);

            ACLMessage response = behavior.receive(template);
            if (response != null) {
                messageSent = false;
                return response;
            }
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    public List<AgentDescription> search(AgentDescription agentDescription) {
        if (!messageSent) {
            ACLMessage msg = new ACLMessage(Performative.REQUEST);
            msg.setSender(behavior.getAgent().getAgentDescription().getAgentid());
            msg.setReceiver(Collections.singletonList(new AgentID(Defines.DEFAULT_EMA_NAME, behavior.getAgent().mts.getEmaAddress())));
            msg.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            msg.setContent(AgentManagementOntology.createAgentDescriptionMessageContent(
                    Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION,
                    Collections.singletonList(agentDescription)
            ));
            msg.setConversationId(conversationID);

            sendToEMA(msg);
            messageSent = true;
        }

        if (messageSent) {
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setSender(new AgentID(Defines.DEFAULT_EMA_NAME));
            template.setPerformatives(Collections.singleton(Performative.INFORM));
            template.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            template.setConversationId(conversationID);

            ACLMessage response = behavior.receive(template);
            if (response != null) {
                messageSent = false;
                return AgentManagementOntology.getAgentDescriptionsFromAttributes((java.util.Map<String, Object>) response.getContent());
            }
        }
        return null;
    }
    @SuppressWarnings("unchecked")
    public List<AgentDescription> localSearch(AgentDescription agentDescription) {
        if (!messageSent) {
            ACLMessage msg = new ACLMessage(Performative.REQUEST);
            msg.setSender(behavior.getAgent().getAgentDescription().getAgentid());
            msg.setReceiver(Collections.singletonList(new AgentID(Defines.DEFAULT_EMA_NAME, behavior.getAgent().mts.getEmaAddress())));
            msg.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            msg.setContent(AgentManagementOntology.createAgentDescriptionMessageContent(
                    Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION,
                    Collections.singletonList(agentDescription)
            ));
            msg.setConversationId(conversationID);

            sendToEMA(msg);
            messageSent = true;
        }

        if (messageSent) {
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setSender(new AgentID(Defines.DEFAULT_EMA_NAME));
            template.setPerformatives(Collections.singleton(Performative.INFORM));
            template.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            template.setConversationId(conversationID);

            ACLMessage response = behavior.receive(template);
            if (response != null) {
                messageSent = false;
                return AgentManagementOntology.getAgentDescriptionsFromAttributes((java.util.Map<String, Object>) response.getContent());
            }
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    public List<AgentDescription> externalSearch(AgentDescription agentDescription) {
        if (!messageSent) {
            ACLMessage msg = new ACLMessage(Performative.REQUEST);
            msg.setSender(behavior.getAgent().getAgentDescription().getAgentid());
            msg.setReceiver(Collections.singletonList(new AgentID(Defines.DEFAULT_EMA_NAME, behavior.getAgent().mts.getEmaAddress())));
            msg.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            msg.setContent(AgentManagementOntology.createAgentDescriptionMessageContent(
                    Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION,
                    Collections.singletonList(agentDescription)
            ));
            msg.setConversationId(conversationID);

            sendToEMA(msg);
            messageSent = true;
        }

        if (messageSent) {
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setSender(new AgentID(Defines.DEFAULT_EMA_NAME));
            template.setPerformatives(Collections.singleton(Performative.INFORM));
            template.setOntology(Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
            template.setConversationId(conversationID);

            ACLMessage response = behavior.receive(template);
            if (response != null) {
                messageSent = false;
                return AgentManagementOntology.getAgentDescriptionsFromAttributes((java.util.Map<String, Object>) response.getContent());
            }
        }
        return null;
    }
}
