/* --------------------------------------------
 * Name: Charuni Liyanage
 * Purpose: This program sets up a P2P chat app where nodes are sent and received   
 *          The user is prompted to change their node id or direct their message or 
 *          broadcast it. Invalid error checks are also placed.
 * COMMENT: *** ADOPTED FROM THE Tutorials EXAMPLE DEMO ***
 -----------------------------------------------*/

// include all neccessary libraries 
#include "sysio.h"
#include "serf.h"
#include "ser.h"
#include "phys_cc1350.h"
#include "plug_null.h"
#include "tcv.h"
#include "tcvphys.h"

// Set buffer size
#define CC1350_BUF_SZ 250

// A. Message Structure:
struct msg {
    byte senderId; // 1 byte
    byte receiverId; // 1 byte
    byte sequenceNumber; // 1 byte
    byte payload[27]; // 27 bytes
};

// Define Global Variables 
byte nodeId; 
int sequence = 0;

/* session descriptor for the single VNETI session */
int sfd;

// --------------------- B. Program Operation ---------------------------------
/* 
 *  Purpose: Define a finiste state machine for receiving and processing messages.
*/
fsm receiver {
    // Address of the received packet 
    address packet;
    // Pointer to the received message
    struct msg * receivedPtr;

    /*
     * Purpose: State for waiting to receive a packet
    */
    state Receiving:
        // Receive a packet
        packet = tcv_rnp(Receiving, sfd);
    
    /*
     * Purpose:  State for processing a received message.
    */
    state Receive_Msg:
        //Get the pointer to the received message
        receivedPtr = (struct msg *)(packet + 1);

        // Check if the message is directed to this node
        if(receivedPtr->receiverId == nodeId) {
            proceed Direct; // Proceed to handling direct message
        } else if (receivedPtr->receiverId == '0' || receivedPtr->receiverId == 0) {
            proceed Broadcast; // Proceed to handling broadcast message
        }
        // Continue receiving if message is not for this node
        proceed Receiving;
    
    /*
     * Purpose: State for handling a direct message.
    */
    state Direct:
        ser_outf(Receive_Msg, "Message ");
        proceed Show_Message;
    
    /*
     * Purpose: State for handling a broadcast message.
    */
    state Broadcast:
        ser_outf(Broadcast, "Broadcast ");

    /*
     * Purpose: State for displaying the received message.
    */
    state Show_Message:
        ser_outf(Show_Message, "Message from node %d (Seq %d): %s\n\r", receivedPtr->senderId, receivedPtr->sequenceNumber, receivedPtr->payload);
        tcv_endp(packet);
        // Return to receiving state for more messages
        proceed Receiving;
}

/*
 * Purpose: Finite state machine for sending messages.
*/
fsm send(struct msg * ptr) {
    /*
     * Purpose: State for sending a message.
    */
    state Send_Msg:
        // Create a new packet to send
        address spkt = tcv_wnp(Send_Msg, sfd, sizeof(struct msg) + 4);
        
        // Initialize the packet
        spkt [0] = 0;
        byte * p = (byte*)(spkt + 1);

        // Populate the packet with message data
        *p = ptr->senderId; p++;
        *p = ptr->receiverId; p++;
        *p = ptr->sequenceNumber; p++;
        strcpy(p, ptr->payload);

        // Increment the message sequence number
        sequence++;

        // Output a confirmation message
        tcv_endp (spkt);
        ser_outf(Send_Msg, "\n\rMessage Sent\n\r");

        // Finish the state machine
        finish;
}

/*
 * Purpose:  Root state machine for managing the P2P chat application.
*/
fsm root {
    byte receiverId;
    struct msg * ptr;

    /*
     * Purpose: Initialization state to set up the application.
    */
    state INIT:
        // Set node ID to default value and reset sequence
        nodeId = 1;
        sequence = 0;
        // Allocate memory for the message
        ptr = (struct Message *) umalloc(sizeof(struct msg));
        // Set up cc1350 board
        phys_cc1350(0, CC1350_BUF_SZ);

        // Load null plug-in
        tcv_plug(0, &plug_null);

        // Open the TCV session
        sfd = tcv_open(NONE, 0, 0);
		
        // Check if the session opened successfully
        if (sfd < 0) {
            diag("unable to open TCV session");
            syserror(EASSERT, "no session");
        }

        // Enable physical options and run receiver state machine
        tcv_control(sfd, PHYSOPT_ON, NULL);
        runfsm receiver;

    /*
     * Purpose: State to display the main menu.
    */
    state Menu:
        // Reset receiver ID
        receiverId = 0;
        // Display the menu
        ser_outf(Menu, "\n\rP2P Chat (Node #%d)\n\r"
                       "(C)hange node ID\n\r"
                       "(D)irect transmission\n\r"
                       "(B)roadcast transmission\n\r"
                       "Selection: ", nodeId);
    /*
     * Purpose: State to handle user input choice.
    */
    state Choice:
        // Get user choice
        char choice;
        ser_inf(Choice, "%c", &choice);
        choice = toupper((unsigned char)choice);

        switch (choice) {
            // Change node ID
            case 'C':
                proceed Change_ID;
                break;

            // Direct transmission
            case 'D':
                proceed Direct_Transmission;
                break;
            
            // Broadcast transmission
            case 'B':
                proceed Broadcast_Transmission;
                break;
            // Display error message for incorrect option
            default:
                ser_outf(Choice, "\n\rIncorrect Option.");
                // Return to the main menu
                proceed Menu;
                break;
        }
    /*
     * Purpose: State to prompt user to enter a new node ID.
    */
    state Change_ID:
        ser_outf(Change_ID, "\n\rNew node ID (1-25):");

    /*
     * Purpose: State to get and validate the new node ID entered by the user.
    */
    state Get_ChangeID:
        ser_inf(Get_ChangeID, "%d", &nodeId);
            // Check if the entered node ID is valid
            if (nodeId < 1 || nodeId > 25) {
                ser_outf(Get_ChangeID, "\n\rInvalid ID");
                // Retry getting a valid ID
                proceed Change_ID;
            }
            // Return to the main menu after successful ID change
            proceed Menu;

    /*
     * Purpose: State to prompt user to enter the receiver node ID for direct transmission.
    */
    state Direct_Transmission:
        ser_outf(Direct_Transmission, "\n\rReceiver node ID (1-25):");
    
    /*
     * Purpose: State to get and validate the receiver node ID entered by the user.
    */
    state Get_ReceiverID:
        ser_inf(Get_ReceiverID, "%d", &receiverId);
            // Check if the entered receiver ID is valid
            if (receiverId < 1 || receiverId > 25) {
                // Display error message for invalid ID
                ser_outf(Get_ReceiverID, "\n\rInvalid ID");
                // Retry getting a valid receiver ID
                proceed Direct_Transmission;
            }

    /*
     * Purpose: State to prompt user to enter the message for broadcast transmission.
    */
    state Broadcast_Transmission:
        ser_outf(Broadcast_Transmission, "\n\rMessage: ");
    
    /*
     * Purpose: State to receive and process the message entered by the user.
    */
    state Receive_Msg:
        ser_in(Receive_Msg, ptr->payload, 27);
        if(strlen(ptr->payload) >= 27) {
            // Ensure message is null-terminated
            ptr->payload[26] = '\0';
        }

    /*
     * Purpose: State to send the message after receiving input and validating receiver ID.
    */
    state Sending:
        // Set sender ID
        ptr->senderId = nodeId;
        // Set receiver ID
        ptr->receiverId = receiverId;
        // Set sequence number
        ptr->sequenceNumber = (byte)sequence;
        // Call send finite state machine to transmit the message
        call send(ptr, Menu);
}
