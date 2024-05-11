#include "BlueCarCAN.h"
// Asegúrate de que todas las definiciones y configuraciones necesarias están aquí.

BlueCarCAN::BlueCarCAN(int pinCS, int pinINT, const String nodeNames[], unsigned short int numNodes) : CAN0(pinCS), _pinCS(pinCS), _pinINT(pinINT) {
    pinMode(_pinINT, INPUT);
    SPI.begin();

    if (nodeNames != nullptr && numNodes > 0) {
        node_names = new String[numNodes];
        System_Data = new unsigned int[numNodes];
        num_nodes = numNodes;
        for (int i = 0; i < numNodes; i++) {
            node_names[i] = nodeNames[i];
            System_Data[i] = 0; 
        }
    } else {
        static const String defaultNodeNames[] = {"Direccion", "Propulsion", "Radar"};
        static const unsigned short int defaultNumNodes = 3;
        node_names = new String[defaultNumNodes];
        System_Data = new unsigned int[defaultNumNodes];
        num_nodes = defaultNumNodes;
        for (int i = 0; i < defaultNumNodes; i++) {
            node_names[i] = defaultNodeNames[i];
            System_Data[i] = 0;
        }
    }
}

BlueCarCAN::~BlueCarCAN() {
    delete[] node_names;
    delete[] System_Data;
}

void BlueCarCAN::config_CAN(int config) {
    if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
        Serial.println("MCP2515 Initialized Successfully!");
    } else {
        Serial.println("Error Initializing MCP2515...");
    }
    CAN0.setMode(config == 0 ? MCP_NORMAL : MCP_LOOPBACK);
}
int BlueCarCAN::sendCanMessage(unsigned long id, unsigned char len, unsigned char *data, int attempts) {
    byte sndStat = CAN0.sendMsgBuf(id, 0, len, data); // Asegúrate de que CAN0 está correctamente inicializado y configurado
    if (sndStat == CAN_OK) {
        //Serial.println("Message Sent Successfully!");
        return 1;  // Indica éxito
    } else {
        //Serial.print("Error Sending Message. Attempts remaining: ");
        //Serial.println(attempts);
        if (attempts > 0) {
            return sendCanMessage(id, len, data, attempts - 1);  // Recursivamente reintenta enviar el mensaje
        } else {
            //Serial.println("Failed to send message after several attempts.");
            return 0;  // Indica falla después de agotar los intentos
        }
    }
}

void BlueCarCAN::receiveCanMessage() {
    if (!digitalRead(_pinINT)) {
        CAN0.readMsgBuf(&globalRxId, &globalRxLen, globalRxBuf);
    }
}
int BlueCarCAN::map_name2nodeid(String node_name) {
    for (int i = 0; i < num_nodes; i++) {
        if (node_names[i] == node_name) {
            return i + 1; // Los índices de nodos comienzan en 1
        }
    }
    return 0; // Retorna 0 si no se encuentra el nombre
}

void BlueCarCAN::config_node(String node_name, int config_can) {
    Serial.begin(115200);
    node_arduino = map_name2nodeid(node_name);
    can_id_frame_envio = (node_arduino * 2) - 1;
    can_id_frame_lectura = node_arduino * 2;
    myData();
    config_CAN(config_can);
}

void BlueCarCAN::is4Me() {
    receiveCanMessage();

    if (globalRxId == can_id_frame_lectura) {
        for (int i = 0; i < 8; i++) {
            Serial.println("Yes :D");
            read_message[i] = globalRxBuf[i];
        }
    }
}

void BlueCarCAN::myData() {
    Serial.print("can_id_frame_envio ");
    Serial.print(can_id_frame_envio);
    Serial.print(" can_id_frame_lectura ");
    Serial.print(can_id_frame_lectura);
}

void BlueCarCAN::send2Jetson() {
    
    if (globalRxId % 2 == 1) {
        System_Data[((globalRxId + 1) / 2) - 1] = globalRxBuf[0];
    }
    
    json = "{";
    for (int i = 0; i < num_nodes; i++) {
        json += "{\"" + String(node_names[i]) + "\":" + String(System_Data[i]) + "}";
        if (i < num_nodes - 1) {
            json += ", ";
        }
    }
    json += "}";
}

void BlueCarCAN::readJetson() {
    int key = 0;
    int value = 0;
    int pos = 1;
    while (pos < json_received.length() - 1) {
        int colonPos = json_received.indexOf(':', pos);
        if (colonPos == -1) break;
        String nodeNameExt = json_received.substring(pos, colonPos);
        key = map_name2nodeid(nodeNameExt);
        int commaPos = json_received.indexOf(',', colonPos);
        if (commaPos == -1) {
            commaPos = json_received.length() - 1;
        }
        value = json_received.substring(colonPos + 1, commaPos).toInt();

        write_message[0] = value;
        if (node_arduino == key) {
            read_message[0] = value;
        }

        sendCanMessage(2 * key, len, write_message);

        pos = commaPos + 2;
    }
}

void BlueCarCAN::setWriteMessage(byte index, byte value) {
    if (index < sizeof(write_message)) {
        write_message[index] = value;
        sendCanMessage(can_id_frame_envio, len, write_message);
    }
}

byte BlueCarCAN::getReadMessage(byte index) {
    if (index < sizeof(read_message)) {
        return read_message[index];
    }
    return 0;  // Return 0 if index is out of bounds
}
