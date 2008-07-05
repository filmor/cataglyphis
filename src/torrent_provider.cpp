#include "torrent_provider.hpp"

#include <iterator>
#include <boost/bind.hpp>

namespace cataglyphis
{

    class torrent_provider::connection 
        : public boost::enable_shared_from_this<torrent_provider::connection>
    {
    public:
        typedef boost::shared_ptr<connection> pointer;
        
        static pointer create (boost::asio::io_service& ios, std::string const& tor)
        {
            return pointer (new connection (ios, tor));
        }

        boost::asio::ip::tcp::socket& socket ()
        {
            return socket_; 
        }

        void start ();

    private:
        connection (boost::asio::io_service& ios, std::string const& tor)
            :  socket_ (ios), torrent_ (tor)
        {}

        void handle_write () const {}

        boost::asio::ip::tcp::socket socket_;
        std::string const& torrent_;
    };

    torrent_provider::torrent_provider (libtorrent::torrent_info const& tor,
                                        boost::asio::io_service& ios)
        : acceptor_ (ios), io_service_ (ios), torrent_ (tor.metadata ().get ())
    {
        /// Initiiere acceptor_
        start_accept ();
    }

    boost::uint16_t torrent_provider::port () const
    {
        return acceptor_.local_endpoint ().port ();
    }

    void torrent_provider::start_accept ()
    {
        connection::pointer new_connection
            = connection::create (io_service_, torrent_);

        acceptor_.async_accept (new_connection->socket (),
                boost::bind (&torrent_provider::handle_accept, this, new_connection,
                boost::asio::placeholders::error)
                );
    }

    void torrent_provider::handle_accept (connection::pointer new_connection,
                                          boost::system::error_code const& error)
    {
        if (!error)
        {
            new_connection->start ();
            start_accept ();
        }
    }

    void torrent_provider::connection::start ()
    {
        boost::asio::async_write (socket_, boost::asio::buffer (torrent_),
                boost::bind (&connection::handle_write, shared_from_this ())
                );
    }

}
