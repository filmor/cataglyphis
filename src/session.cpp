#include "session.hpp"
#include "torrent_provider.hpp"

#include <sstream>
#include <iomanip>

#include <unistd.h>

#include <libtorrent/create_torrent.hpp>
#include <libtorrent/fingerprint.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
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

    class session::provided_torrent
    {
    public:
        provided_torrent (bf::path path, lt::torrent_info info, lt::session& s,
                          boost::asio::io_service& ios)
            : provider_ (info, ios)
            , registerer_ ("cataglyphis", transport::tcp, path.leaf (),
                             s.listen_port (), bignum_to_string (info.info_hash())
                            )
        {
            lt::add_torrent_params p;
            p.ti = &info;
            p.save_path = path;
            handle_ = s.add_torrent (p);
        }

        ~provided_torrent ()
        {
            /// \todo s.delete_torrent ()
        }

    private:
        lt::torrent_handle handle_;
        torrent_provider provider_;
        service_registerer registerer_;
    };

    session::session ()
        : session_ (lt::fingerprint ("CG", 3, 1, 4, 1), std::make_pair (6300, 6400))
    {
        /// \todo
        lt::dht_settings settings;
        settings.service_port = session_.listen_port () + 1;
        session_.set_dht_settings (settings);
        session_.start_dht ();
        session_.add_extension (&lt::create_ut_metadata_plugin);
        session_.add_extension (&lt::create_ut_pex_plugin);
    }

    /// \todo Torrent provider
    void session::put_element (session::path_type const& path)
    {
        lt::file_storage fs;
        fs.set_piece_length (1000);

        fs.add_file (path, bf::file_size (path));

        lt::torrent_info info (lt::create_torrent (fs).generate ());

        try
        {
            registry_.push_back (
                    torrent_ptr (new provided_torrent (path, info, session_, ios_))
                    );
        }
        catch (already_registered const& exc)
        {
            /// \todo
            throw exc;
        }
    }

    void session::get_element (std::string const& name, 
            session::path_type const& path)
    {
        service_record result =
            service ("cataglyphis", transport::tcp, name).get_record ();

        DEBUG_OUT(result.port);

        session_.add_dht_node (std::make_pair (result.host, result.port));

        DEBUG_OUT(session_.dht_state ());

        lt::sha1_hash info_hash = string_to_bignum (result.txt_record["hash"]);
        DEBUG_OUT(bignum_to_string(info_hash));
        session_.add_torrent ("", info_hash, name.c_str (), path);

        session_.set_severity_level (libtorrent::alert::debug);

        while (true)
        {
            std::auto_ptr<lt::alert> a = session_.pop_alert ();
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

