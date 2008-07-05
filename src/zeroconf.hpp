#ifndef CATAGLYPHIS_ZEROCONF_HPP
#define CATAGLYPHIS_ZEROCONF_HPP

#include <exception>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/ip/address.hpp>

#include <boost/preprocessor/stringize.hpp>

#define DEBUG_OUT(expr) \
    std::cout << BOOST_PP_STRINGIZE(expr) << ": " << (expr) << std::endl;

namespace cataglyphis
{

    struct zeroconf_exception : public std::exception {};
    struct already_registered : public zeroconf_exception {};
    struct service_not_found : public zeroconf_exception {};

    struct transport
    {
        enum type
        {
            tcp,
            udp
        };
    };

    template <typename InIt>
    std::string serialize_txt (InIt begin, InIt end)
    {
        std::ostringstream os;
        while (begin != end)
        {
            std::string const& key = begin->first;
            std::string const& value = begin->second;
            os << key.length () << key << value.length () << value;
        }
        return os.str ();
    }

    template <typename InIt, typename OutIt>
    void deserialize_txt (InIt i, InIt end, OutIt out)
    {
        while (i != end)
        {
            std::string::size_type len = static_cast<std::string::size_type> (*i++);
            
            InIt begin = i;
            InIt local_end = i + len;

            for (; i != local_end; ++i)
                if (*i == '=')
                    break;

            (out++) = std::make_pair (std::string (begin, i)
                                     ,std::string (i + 1, local_end)
                                );
            i = local_end;
        }
    }

    struct service_record
    {
        std::string service_type;
        std::string name;
        std::string host;
        boost::uint16_t port;
        std::map<std::string, std::string> txt_record;
    };

    class service
    {
    public:
        friend std::vector<service> browse_services (std::string const& service,
                                                     transport::type transport);

        service (std::string const& service, transport::type transport,
                 std::string const& name);

        std::string const& get_name () const;

        service_record get_record ();

    private:
        class impl;

        service (boost::shared_ptr<impl> i) : _impl (i) {}
        
        boost::shared_ptr<impl> _impl;
    };

    class service_registerer
    {
    public:
        service_registerer (std::string const& type, transport::type transport,
                            std::string const& name, boost::uint16_t port,
                            std::string const& text);

    private:
        class impl;
        boost::shared_ptr<impl> _impl;
    };

    std::vector<service> browse_services (std::string const& service,
                                          transport::type transport);

}

#endif

