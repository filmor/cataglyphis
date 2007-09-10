#include "session.hpp"

#include <sstream>
#include <iomanip>

#include <unistd.h>

#include <libtorrent/fingerprint.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>

#define DEBUG_OUT(expr) \
    std::cout << BOOST_PP_STRINGIZE(expr) << ": " << (expr) << std::endl;

namespace lt = libtorrent;
namespace bf = boost::filesystem;

namespace cataglyphis
{

    namespace
    {
        typedef ::libtorrent::big_number bignum;

        std::string bignum_to_string (bignum const& n)
        {
            std::ostringstream os;
            os.width (2);
            os.fill ('0');
            BOOST_FOREACH(unsigned byte, n)
            {
                os << std::hex <<  byte;
            }
            return os.str ();
        }

        template <typename Iterator>
        unsigned char read_byte (Iterator& i)
        {
            unsigned char c = *i++;
            unsigned char result = (c < 'a' ? c - '0' : c - 'a' + 10) * 16;
            c = *i++;
            return result + (c < 'a' ? c - '0' : c - 'a' + 10);
        }

        bignum string_to_bignum (std::string const& s)
        {
            if (s.length () < 40)
                throw 'a';
            bignum result;
            std::string::const_iterator i = s.begin ();
            BOOST_FOREACH(unsigned char& b, result)
            {
                b = read_byte (i);
            }
            return result;
        }
    }

    session::session ()
        : _session (lt::fingerprint ("CG", 3, 1, 4, 1), std::make_pair (6300, 6400))
    {
        _session.start_dht ();
        _session.add_extension (&lt::create_metadata_plugin);
        _session.add_extension (&lt::create_ut_pex_plugin);
    }

    void session::put_element (session::path_type const& path)
    {
        std::string name = path.leaf ();
        lt::torrent_info info;

        if (bf::is_directory (path))
            return;

        info.add_file (path, bf::file_size(path));

        info.create_torrent ();

        try
        {
            _registry.push_back(
                    service_registerer ("cataglyphis", transport::tcp, name,
                        _session.listen_port (), bignum_to_string (info.info_hash())
                        )
                    );
            info.print (std::cout);
            _session.add_torrent (info, path);
        }
        catch (already_registered const& exc)
        {
            throw exc;
        }
    }

    void session::get_element (std::string const& name, 
            session::path_type const& path)
    {
        service_record result =
            service ("cataglyphis", transport::tcp, name).get_record ();

        DEBUG_OUT(result.port);

        _session.add_dht_node (std::make_pair (result.host,
                                               result.port)
                );

        DEBUG_OUT(_session.dht_state ());

        lt::sha1_hash info_hash = string_to_bignum (result.txt_record);
        DEBUG_OUT(bignum_to_string(info_hash));
        _session.add_torrent ("", info_hash, name.c_str (), path);

        _session.set_severity_level (libtorrent::alert::debug);

        while (true)
        {
            std::auto_ptr<lt::alert> a = _session.pop_alert ();
            if (a.get () != 0)
                DEBUG_OUT(a->msg ());
            ::usleep (1000000);
        }
    }

    namespace
    {
        std::string const& transformer (service& s)
        {
            return s.get_name ();
        }
    }

    std::vector<std::string> session::available_elements () const
    {
        std::vector<service> browse_result = 
            browse_services ("cataglyphis", transport::tcp);

        std::vector<std::string> result (browse_result.size ());

        std::transform (browse_result.begin (), browse_result.end (), result.begin (),
                        &transformer);
        return result;
    }

}

