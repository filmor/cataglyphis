#include "torrent_provider.hpp"

#include <boost/bind.hpp>

namespace cataglyphis
{

    torrent_provider::torrent_provider (libtorrent::torrent_info const& tor,
                                        asio::io_service& ios)
        : _torrent (tor), _acceptor (ios), _io_service (ios)
    {
        /// Initiiere _acceptor
        start_accept ();
    }

    boost::uint16_t torrent_provider::port () const
    {
        return _acceptor.local_endpoint ().port ();
    }

    void torrent_provider::start_accept ()
    {
        connection::pointer new_connection
            = connection::create (_io_service, _torrent.create_torrent ());

        _acceptor.async_accept (new_connection->socket (),
                boost::bind (&torrent_provider::handle_accept, this, new_connection,
                asio::placeholders::error)
                );
    }

    void torrent_provider::handle_accept (connection::pointer new_connection,
                                          asio::error_code const& error)
    {
        if (!error)
        {
            new_connection->start ();
            start_accept ();
        }
    }

    void torrent_provider::connection::start ()
    {
        asio::async_write (_socket, asio::buffer (_torrent),
                boost::bind (&connection::handle_write, shared_from_this ())
                );
    }

}
