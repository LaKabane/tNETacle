#include <iostream>
#include <string>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
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

void
broadcast(std::string ip,
          std::string port,
          std::string tnt_ip,
          std::string tnt_port)
{
    int bite = 0;
    asio::io_service io_service;
    ip::udp::socket s(io_service);
    ip::tcp::iostream tnetacle(tnt_ip, "4243");
    std::stringstream ss;
    pt::ptree request;
    char buff[150];

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
        ip::udp::endpoint sender_endpoint;
        std::stringstream _data;
        std::string val;

        int n = s.receive_from(asio::buffer(buff), sender_endpoint);
        if (bite == 0)
        {
            bite++;
            continue;
        }
        buff[n - 1] = 0;
        val.assign(buff);
        request.put("AddContact.Ip", val);
        request.put("AddContact.Name", "bite"); 
        request.put("AddContact.Key", "bitebitebite"); 
        if (tnetacle) // implicit bool cast, boooouh
        {
            pt::json_parser::write_json(_data, request);
            tnetacle << _data.str() << std::endl;
        }
        buff[n] = 0;
        std::cout << local_endpoint << ": " << buff << std::endl;
    }
}

int
main(int argc, const char *argv[])
{
    po::options_description desc("Allowed options");
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
        broadcast(vm["bip"].as<std::string>(), vm["bport"].as<std::string>(),
                  vm["ip"].as<std::string>(), vm["port"].as<std::string>());
    return (0);
}
