#ifndef CATAGLYPHIS_TORRENT_PROVIDER_HPP
#define CATAGLYPHIS_TORRENT_PROVIDER_HPP

#include <boost/cstdint.hpp>
#include <asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <libtorrent/torrent_info.hpp>

namespace cataglyphis
{

    class torrent_provider
    {
    public:
        using ::libtorrent::torrent_info;

        torrent_provider (torrent_info const&, asio::io_service&);

        boost::uint16_t port () const;

    private:
        class connection : public boost::enable_shared_from_this<connection>
        {
        public:
            typedef boost::shared_ptr<connection> pointer;
            
            static pointer create (asio::io_service& ios, std::string const& tor)
            {
                return pointer (new connection (ios, tor));
            }

            asio::ip::tcp::socket& socket ()
            {
                return _socket; 
            }

            void start ();

        private:
            connection (asio::io_service& ios, std::string const& tor)
                :  _socket (ios), _torrent (tor)
            {}

            void handle_write () const {}

            asio::ip::tcp::socket _socket;
            std::string const& _torrent;
        };

        void start_accept ();
        void handle_accept (boost::shared_ptr<connection>, asio::error_code const&);

        torrent const& _torrent;
        asio::ip::tcp::acceptor _acceptor;
        asio::io_service& _io_service;
    };

}

#endif
