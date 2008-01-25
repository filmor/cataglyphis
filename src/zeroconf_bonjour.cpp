#include "zeroconf.hpp"

#include <sstream>
#include <iterator>
#include <iostream>

#include <boost/preprocessor/stringize.hpp>

#define DEBUG_OUT(expr) \
    std::cout << BOOST_PP_STRINGIZE(expr) << ": " << (expr) << std::endl;

#include <dns_sd.h>

namespace cataglyphis
{

    namespace
    {
        inline std::string to_service_type (std::string const& service,
                                            transport::type tp)
        {
            std::ostringstream os;
            os << '_' << service << "._"
               <<   (tp == transport::udp ? "udp"
                  : (tp == transport::tcp ? "tcp"
                  : throw tp
                  ));
            return os.str ();
        }

    }

    class service::impl
    {
    public:
        impl (std::string const& service_type, std::string name, std::string domain,
              boost::uint32_t interface = 0)
            : _name (name), _domain (domain)
        {
            _service_record.service_type = service_type;
        }

        ~impl ()
        {
            DNSServiceRefDeallocate (_sdref);
        }

        std::string const& get_name () const
        {
            return _name;
        }

        service_record get_record ()
        {
            _error_code =
                DNSServiceResolve (&_sdref, 0, 0, _name.c_str (), 
                                   _service_record.service_type.c_str (),
                                   _domain.c_str (), &impl::_callback,
                                    reinterpret_cast<void*> (this)
                        );

            DNSServiceProcessResult (_sdref);

            return _service_record;
        }

    private:
        static void _callback (DNSServiceRef sdref, DNSServiceFlags,
                boost::uint32_t iface, DNSServiceErrorType error,
                const char* fullname, const char* hosttarget, boost::uint16_t port,
                boost::uint16_t txt_len, const char* txt_record, void* context
                )
        {
            impl* this_ = reinterpret_cast<impl*> (context);
            service_record& result = this_->_service_record;
            this_->_error_code = error;

            // DEBUG_OUT(error);

            if (error)
                return;

            DEBUG_OUT(error);
            DEBUG_OUT(error);

            result.port = port;
            DEBUG_OUT(port);
            result.host = std::string (hosttarget);
            deserialize_txt (
                txt_record, txt_record + txt_len, 
                std::inserter (result.txt_record, result.txt_record.end ())
                );
            DEBUG_OUT(port);
            DEBUG_OUT(result.host);
        }

        DNSServiceRef _sdref;
        DNSServiceErrorType _error_code;
        service_record _service_record;
        std::string _name;
        std::string _domain;
    };

    service::service (std::string const& service, transport::type transport,
                      std::string const& name)
        : _impl (new service::impl (to_service_type (service, transport), name,
                                    "local."))
    {}

    service_record service::get_record ()
    {
        return _impl->get_record ();
    }

    std::string const& service::get_name () const
    {
        return _impl->get_name ();
    }

    class service_registerer::impl
    {
    public:
        impl (const char* service_type, const char* name, boost::uint16_t port,
              std::string const& txt_record)
            : _buffer (new char[txt_record.length () + 1 + sizeof("hash")])
        {
            DEBUG_OUT(txt_record);
            TXTRecordCreate (&_txtref, txt_record.length () + 6, _buffer);
            TXTRecordSetValue (&_txtref, "hash", txt_record.length (),
                               reinterpret_cast<const void*> (txt_record.c_str ())
                    );

            DNSServiceRegister (&_sdref, 0, 0, name, service_type,
                                0, 0, port, TXTRecordGetLength (&_txtref),
                                TXTRecordGetBytesPtr (&_txtref), 
                                &impl::_callback, 0);

            DNSServiceProcessResult (_sdref);
        }

        ~impl ()
        {
            DNSServiceRefDeallocate (_sdref);
            TXTRecordDeallocate (&_txtref);
            delete[] _buffer;
        }

    private:
        static void _callback (DNSServiceRef, DNSServiceFlags,
                DNSServiceErrorType error, const char* name, const char* regtype,
                const char*, void* context
                )
        {
            DEBUG_OUT(error);
        }

        TXTRecordRef _txtref;
        char* _buffer;
        DNSServiceRef _sdref;
    };


    service_registerer::service_registerer (std::string const& service,
                                transport::type transport, std::string const& name,
                                boost::uint16_t port, std::string const& txt_record)
        : _impl (new impl(to_service_type (service, transport).c_str (),
                          name.c_str (), port, txt_record)
                )
    {}

    
    std::vector<service> browse_services (std::string const& service_name,
                                                 transport::type transport)
    {
        struct impl
        {
            static void callback (DNSServiceRef sdref, DNSServiceFlags flags,
                    boost::uint32_t iface, DNSServiceErrorType error_code,
                    const char* name, const char* reg_type, const char* domain,
                    void* context)
            {
                std::vector<service>& result =
                    *reinterpret_cast<std::vector<service>*> (context);
                result.push_back (service (boost::shared_ptr<service::impl> (
                                new service::impl (std::string (reg_type),
                                                   std::string (name),
                                                   std::string (domain),
                                                   iface)
                                )
                            )
                        );
                DEBUG_OUT(reg_type);
                DEBUG_OUT(name);
                DEBUG_OUT(domain);

                if (flags & kDNSServiceFlagsMoreComing)
                    DNSServiceProcessResult (sdref);
            }
        };

        std::vector<service> result;

        DNSServiceRef sdref;

        DNSServiceBrowse (&sdref, 0, 0,
                          to_service_type (service_name, transport).c_str (),
                          0, &impl::callback, reinterpret_cast<void*> (&result));

        DNSServiceProcessResult (sdref);
        DNSServiceRefDeallocate (sdref);

        DEBUG_OUT(result.size ());

        return result;
    }

}

