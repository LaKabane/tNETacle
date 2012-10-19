#include <iostream>
#include <string>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <boost/program_options.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "tclt.h"

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace po = boost::program_options;
namespace pt = boost::property_tree;

static const std::string DEFAULT_IP = "255.255.255.255";
static const std::string DEFAULT_PORT = "3456";

static const std::string DEFAULT_TNT_IP = "127.0.0.1";
static const std::string DEFAULT_TNT_PORT = "4242";

class broadcaster
{
    ip::udp::socket     broadcast;
    ip::udp::endpoint   local_endpoint;
    ip::udp::endpoint   sender_endpoint;

    ip::tcp::iostream   tnetacle;
    ip::tcp::endpoint   tnetacle_endpoint;

    asio::deadline_timer    announce;

    std::vector<char>  buffer;
    pt::ptree                   request;
 public:
    broadcaster(asio::io_service &io_service,
                std::string ip,
                std::string port,
                std::string tnt_ip,
                std::string tnt_port)
        : broadcast(io_service)
        , local_endpoint(ip::address::from_string(ip), std::atoi(port.c_str()))
        , sender_endpoint{}
        , tnetacle(tnt_ip, "4243")
        , tnetacle_endpoint(ip::address::from_string(tnt_ip), std::atoi(tnt_port.c_str()))
        , announce(io_service)
        , buffer(150)
        , request{}
    {
        broadcast.open(ip::udp::v4());
        broadcast.set_option(asio::socket_base::broadcast(true));
        broadcast.set_option(asio::socket_base::reuse_address(true));

        std::cout << "local endpoint: " << local_endpoint << std::endl;
        std::cout << "tnetalce endpoint: " << tnetacle_endpoint << std::endl;
        
        broadcast.bind(local_endpoint);
    }

    void handl_recv(boost::system::error_code const &err, size_t bytes)
    {
        this->request.put("AddContact.Ip", this->buffer.data());
        this->request.put("AddContact.Name", "bite"); 
        this->request.put("AddContact.Key", "bitebitebite"); 

        if (this->tnetacle)
        {
            std::string data;
            std::string tnetacle_pres;

            data.assign(this->buffer.data());
            tnetacle_pres.assign(this->tnetacle_endpoint.address().to_string());

            if (data.find(tnetacle_pres) == std::string::npos)
            {
                std::stringstream data;

                pt::json_parser::write_json(data, this->request);
                this->tnetacle << data.str();
                std::cerr << "send to [" << this->tnetacle_endpoint << "]: "
                    << data.str();
            }
        }
        this->broadcast.async_receive_from(asio::buffer(this->buffer),
                                           this->sender_endpoint,
                                           [this](boost::system::error_code const &err, size_t bytes) {
                                               if (!err)
                                                   this->handl_recv(err, bytes);
                                           });
    }

    void handle_announce(boost::system::error_code const &err)
    {
        if (!err)
        {
            std::stringstream ss;

            ss << this->tnetacle_endpoint;
            this->broadcast.send_to(asio::buffer(ss.str()), local_endpoint);
            this->announce.expires_from_now(boost::posix_time::seconds(5));
            this->announce.async_wait([this](boost::system::error_code const &err)
                                      {
                                          this->handle_announce(err);
                                      });
        }
    }

    void start()
    {
        std::stringstream ss;

        ss << this->tnetacle_endpoint;
        this->broadcast.send_to(asio::buffer(ss.str()), local_endpoint);
        this->broadcast.async_receive_from(asio::buffer(this->buffer),
                                           this->sender_endpoint,
                                           [this](boost::system::error_code const &err, size_t bytes)
                                           {
                                               if (!err)
                                                   this->handl_recv(err, bytes);
                                           });
        this->announce.expires_from_now(boost::posix_time::seconds(5));
        this->announce.async_wait([this](boost::system::error_code const &err)
                                  {
                                      this->handle_announce(err);
                                  });
    }

};

void
broadcast(std::string ip,
          std::string port,
          std::string tnt_ip,
          std::string tnt_port)
{
    asio::io_service io_service;
    ip::udp::socket s(io_service);
    ip::tcp::iostream tnetacle(tnt_ip, "4243");
    std::stringstream ss;
    pt::ptree request;

    s.open(ip::udp::v4());
    s.set_option(asio::socket_base::broadcast(true));
    s.set_option(asio::socket_base::reuse_address(true));

    ip::udp::endpoint local_endpoint(ip::address_v4::from_string(ip),
                                     std::atoi(port.c_str()));
    ip::udp::endpoint tnetacle_endpoint(ip::address_v4::from_string(tnt_ip),
                                        std::atoi(tnt_port.c_str()));

    std::cout << local_endpoint << std::endl;
    std::cout << tnetacle_endpoint << std::endl;
    ss << tnetacle_endpoint;
    s.bind(local_endpoint);
    s.send_to(asio::buffer(ss.str()), local_endpoint);
    while (1)
    {
        char buff[150];
        ip::udp::endpoint sender_endpoint;
        std::stringstream _data;
        std::string val;

        int n = s.receive_from(asio::buffer(buff), sender_endpoint);
        buff[n] = 0;
        val.assign(buff);
        request.put("AddContact.Ip", val);
        request.put("AddContact.Name", "bite"); 
        request.put("AddContact.Key", "bitebitebite"); 
        if (tnetacle) // implicit bool cast, boooouh
        {
            pt::json_parser::write_json(_data, request);
            tnetacle << _data.str() << std::endl;
        }
        std::cout << local_endpoint << ": " << buff << std::endl;
    }
}

int
main(int argc, char *argv[])
{
    po::options_description desc("Allowed options");
    asio::io_service io_service;
    po::variables_map vm;

    desc.add_options()
        ("help", "show this message")
        ("ip", po::value<std::string>()->default_value(DEFAULT_TNT_IP),
             "set the tNETacle node ip")
        ("port", po::value<std::string>()->default_value(DEFAULT_TNT_PORT),
             "set the tNETacle port")
        ("bip", po::value<std::string>()->default_value(DEFAULT_IP),
             "set the broadcast ip")
        ("bport", po::value<std::string>()->default_value(DEFAULT_PORT),
             "set the broadcast ip")
    ;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") > 0)
    {
        std::cout << desc;
    }
    else
    {
        broadcaster b(io_service,
                      vm["bip"].as<std::string>(),
                      vm["bport"].as<std::string>(),
                      vm["ip"].as<std::string>(),
                      vm["port"].as<std::string>());
       b.start();
       io_service.run();
    }
    return (0);
}
