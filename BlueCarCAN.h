#ifndef BlueCarCAN_h
#define BlueCarCAN_h

#include <mcp_can.h>
#include <SPI.h>

class BlueCarCAN {
public:
    String json_received, json;
    BlueCarCAN(int pinCS, int pinINT, const String nodeNames[] = nullptr, unsigned short int numNodes = 0);
    ~BlueCarCAN(); // Destructor para manejar la limpieza de memoria
    void config_CAN(int config = 0);
    int sendCanMessage(unsigned long id, unsigned char len, unsigned char *data, int attempts = 3);
    void receiveCanMessage();
    void config_node(String node_name, int config_can = 0);
    void is4Me(int debug = 0);
    void myData();
    void send2Jetson();
    void readJetson(int debbug = 0);
    void setWriteMessage(byte index, byte value);
    int map_name2nodeid(String node_name);
    byte getReadMessage(byte index);
private:
    MCP_CAN CAN0;
    int _pinCS;
    int _pinINT;
    String *node_names; // Dinámicamente asignado basado en la entrada del usuario
    unsigned short int num_nodes; // Número de nodos

    byte write_message[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char rxBuf[8];
    unsigned short int read_message[8];
    unsigned char globalRxBuf[8];
    unsigned int *System_Data; // Dinámicamente asignado basado en num_nodes

    unsigned short int node_arduino = 0;
    unsigned short int can_id_frame_envio, can_id_frame_lectura;
   

    unsigned char len = sizeof(write_message) / sizeof(write_message[0]);
    long unsigned int rxId;
    unsigned long globalRxId;
    unsigned char globalRxLen;
};

#endif
