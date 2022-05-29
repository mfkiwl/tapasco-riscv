#ifndef DM_INTERFACE_HPP
#define DM_INTERFACE_HPP

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <thread>
#include <string>
#include <sstream>
#include <queue>
#include <optional>
#include <mutex>
#include <iostream>
#include <sstream>
#include <cassert>
#include <condition_variable>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "tapasco-riscv.capnp.h"
#include "DMI_Types.hpp"


namespace dm
{
    class RequestResponseFIFO
    {
    private:
        std::condition_variable request_cv;
        std::mutex request_queue_mutex;
        std::queue<Request> request_queue;

        std::condition_variable response_cv;
        std::mutex response_queue_mutex;
        std::queue<Response> response_queue;
    public:
        /* server side */
        void push_request(const Request& req); /* blocking */
        std::optional<Response> pop_response();
        bool has_response();
        void wait_for_response(const volatile bool& early_abort_neg);
        /* client side */
        void push_response(const Response& resp); /* blocking */
        std::optional<Request> pop_request(); /* non-blocking */
        bool has_requests();
        void wait_for_request(const volatile bool &early_abort_neg);
    };

    /* This can later be extended to forwarding and test bench connection. */
    class DM_Interface
    {
    protected:
        static std::string req_to_string(const Request& req)
        {
            std::stringstream ss;

            if (req.isRead)
                ss << "READ ";
            else
                ss << "WRITE ";

            switch (req.type) {
                case Request_RequestType_dtm:
                    ss << "DTM";
                    break;
                case Request_RequestType_dm:
                    ss << "DM";
                    break;
                case Request_RequestType_register:
                    ss << "REG";
                    break;
                case Request_RequestType_memory:
                    ss << "MEM";
                    break;
                case Request_RequestType_systemBus:
                    ss << "BUS";
                    break;
                case Request_RequestType_control:
                    ss << "CTRL";
                    break;
                default:
                    return "INVALID";
            }

            ss << " from/to " << std::hex << req.addr;

            if (!req.isRead)
                ss << " value " << req.data;
            
            return ss.str();
        }

        static Response invalid(const Request& req)
        {
            return Response{.isRead = req.isRead, .success = 0};
        }

        static Response valid(const Request& req, uint32_t data = 0)
        {
            return Response{.isRead = req.isRead, .data = data, .success = 1};
        }

        /* server requests/responses */
        std::shared_ptr<RequestResponseFIFO> fifo;

        Response process_dtm(const Request& req);
        Response process_dm(const Request& req);
        Response process_control(const Request& req);
    public:
        DM_Interface(const std::shared_ptr<RequestResponseFIFO>& fifo);

        /* This will be connected to the test bench at the end of the day. */
        virtual uint32_t read_dm(uint32_t addr) = 0;
        virtual void write_dm(uint32_t addr, uint32_t data) = 0;

        /* immediate response */
        //Response process_request(const Request& req);
    };

    /* This handles the socket to which OpenOCD connects. It takes a client connection, reads and answers requests. */
    class OpenOCDServer
    {
    private:
        const char *socket_file;

        struct sockaddr_un addr_str;
        int socket_fd;

        volatile bool run_server = true;
        std::thread listen_thread;

        std::shared_ptr<RequestResponseFIFO> fifo;

        void handle_connection(int connection_fd);
        void do_listen();
    public:
        OpenOCDServer(const char *socket_path, const std::shared_ptr<RequestResponseFIFO>& fifo);
        ~OpenOCDServer();

        void start_listening();
        void stop_listening();
    };

    /*
        This is an internal buffer for logging and easily accessing the DTM/DM registers, memory, system bus and more.
        The internal state can in the future be forwared to a testbench simulating a core or via DMA to a core on actual
        hardware.
     */
    struct DM_RegisterFile
    {
        uint32_t _unused_0[4];
        uint32_t DM_ABSTRACT_DATA_0;
        uint32_t DM_ABSTRACT_DATA_1;
        uint32_t DM_ABSTRACT_DATA_2;
        uint32_t DM_ABSTRACT_DATA_3;
        uint32_t DM_ABSTRACT_DATA_4;
        uint32_t DM_ABSTRACT_DATA_5;
        uint32_t DM_ABSTRACT_DATA_6;
        uint32_t DM_ABSTRACT_DATA_7;
        uint32_t DM_ABSTRACT_DATA_8;
        uint32_t DM_ABSTRACT_DATA_9;
        uint32_t DM_ABSTRACT_DATA_10;
        uint32_t DM_ABSTRACT_DATA_11;
        uint32_t DM_DEBUG_MODULE_CONTROL;
        uint32_t DM_DEBUG_MODULE_STATUS;
        uint32_t DM_HART_INFO;
        uint32_t DM_HART_SUMMARY_1;
        uint32_t DM_HART_ARRAY_WINDOW_SELECT;
        uint32_t DM_HART_ARRAY_WINDOW;
        uint32_t DM_ABSTRACT_CONTROL_AND_STATUS;
        uint32_t DM_ABSTRACT_COMMAND;
        uint32_t DM_ABSTRACT_COMMAND_AUTOEXEC;
        uint32_t DM_CONFIGURATION_STRING_POINTER_0;
        uint32_t DM_CONFIGURATION_STRING_POINTER_1;
        uint32_t DM_CONFIGURATION_STRING_POINTER_2;
        uint32_t DM_CONFIGURATION_STRING_POINTER_3;
        uint32_t DM_NEXT_DEBUG_MODULE;
        uint32_t _unused_1[2];
        uint32_t DM_PROGRAM_BUFFER_0;
        uint32_t DM_PROGRAM_BUFFER_1;
        uint32_t DM_PROGRAM_BUFFER_2;
        uint32_t DM_PROGRAM_BUFFER_3;
        uint32_t DM_PROGRAM_BUFFER_4;
        uint32_t DM_PROGRAM_BUFFER_5;
        uint32_t DM_PROGRAM_BUFFER_6;
        uint32_t DM_PROGRAM_BUFFER_7;
        uint32_t DM_PROGRAM_BUFFER_8;
        uint32_t DM_PROGRAM_BUFFER_9;
        uint32_t DM_PROGRAM_BUFFER_10;
        uint32_t DM_PROGRAM_BUFFER_11;
        uint32_t DM_PROGRAM_BUFFER_12;
        uint32_t DM_PROGRAM_BUFFER_13;
        uint32_t DM_PROGRAM_BUFFER_14;
        uint32_t DM_PROGRAM_BUFFER_15;
        uint32_t DM_AUTHENTICATION_DATA;
        uint32_t _unused_2[3];
        uint32_t DM_HALT_SUMMARY_2;
        uint32_t DM_HALT_SUMMARY_3;
        uint32_t _unused_3[1];
        uint32_t DM_SYSTEM_BUS_ADDRESS_3;
        uint32_t DM_SYSTEM_BUS_ACCESS_CONTROL_AND_STATUS;
        uint32_t DM_SYSTEM_BUS_ADDRESS_0;
        uint32_t DM_SYSTEM_BUS_ADDRESS_1;
        uint32_t DM_SYSTEM_BUS_ADDRESS_2;
        uint32_t DM_SYSTEM_BUS_DATA_0;
        uint32_t DM_SYSTEM_BUS_DATA_1;
        uint32_t DM_SYSTEM_BUS_DATA_2;
        uint32_t DM_SYSTEM_BUS_DATA_3;
        uint32_t DM_HALT_SUMMARY_0;
    } __attribute__((packed));

    struct DTM_RegisterFile
    {

    } __attribute__((packed));
}


#endif /* DM_INTERFACE_HPP */