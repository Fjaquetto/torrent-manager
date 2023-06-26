#include <iostream>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <thread>

int main()
{
    std::string magnetLink;
    std::cout << "Please enter a magnet link: ";
    std::getline(std::cin, magnetLink);

    lt::session session;

    lt::add_torrent_params atp;
    lt::error_code ec;
    lt::parse_magnet_uri(magnetLink, atp, ec);
    if (ec) {
        std::cerr << "Failed to parse magnet link: " << ec.message() << std::endl;
        return 1;
    }

    lt::torrent_handle h = session.add_torrent(atp);

    std::cout << "Downloading Torrent..." << std::endl;

    for (;;) {
        std::vector<lt::alert*> alerts;
        session.pop_alerts(&alerts);

        for (lt::alert const* a : alerts) {
            if (auto at = lt::alert_cast<lt::add_torrent_alert>(a)) {
                std::cout << "Received metadata. Starting torrent download." << std::endl;
            }
            else if (auto st = lt::alert_cast<lt::state_update_alert>(a)) {
                if (st->status.empty()) continue;

                lt::torrent_status const& s = st->status[0];

                if (!s.handle.is_valid()) {
                    std::cout << "Failed to add torrent. Exiting..." << std::endl;
                    return 1;
                }
                else if (s.is_seeding) {
                    std::cout << "Download complete." << std::endl;
                    return 0;
                }
                else {
                    std::cout << "Progress: " << (s.progress_ppm / 10000) << "% " << std::endl;
                    std::cout << "Download speed: " << s.download_rate / 1000.0 << " KB/s" << std::endl;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
        session.post_torrent_updates();
    }

    return 0;
}
