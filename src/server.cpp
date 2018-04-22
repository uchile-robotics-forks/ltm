#include <ltm/server.h>
#include <boost/scoped_ptr.hpp>

using warehouse_ros::Metadata;


namespace ltm {

    Server::Server() {
        ros::NodeHandle priv("~");

        _db_name = "ltm_db";
        _db_collection_name = "episodes";
        _db_host = "localhost";
        _db_port = 27017;
        _db_timeout = 60.0;

        setup_db();

        // Announce services
        _get_episode_service = priv.advertiseService("get_episode", &Server::get_episode_service, this);
        _update_episode_service = priv.advertiseService("update_episode", &Server::update_episode_service, this);
        _add_episode_service = priv.advertiseService("add_episode", &Server::add_episode_service, this);
        _status_service = priv.advertiseService("status", &Server::status_service, this);
        _drop_db_service = priv.advertiseService("drop_db", &Server::drop_db_service, this);

        ROS_INFO("LTM server is up and running.");
        show_status();
    }

    Server::~Server() {}

    void Server::setup_db() {
        // setup DB
        try {
            // host, port, timeout
            _conn.setParams(_db_host, _db_port, _db_timeout);
            _conn.connect();
            _coll = _conn.openCollectionPtr<ltm::Episode>(_db_name, _db_collection_name);
        }
        catch (const warehouse_ros::DbConnectException& exception) {
            // Connection timeout
            ROS_ERROR_STREAM("Connection timeout to DB '" << _db_name << "'.");
            ros::shutdown();
            exit(1);
        }
        // Check for empty database
        if (!_conn.isConnected() || !_coll) {
            ROS_ERROR_STREAM("Connection to DB failed for collection '" << _db_collection_name << "'.");
            ros::shutdown();
            exit(1);
        }
    }

    void Server::show_status() {
        ROS_INFO_STREAM("DB has " << _coll->count() << " entries.");
    }

    std::string Server::to_short_string(const ltm::Episode &episode) {
        std::stringstream ss;
        ss << "<"
           << "uid:" << episode.uid << ", "
           << "type:" << (int) episode.type << ", "
           << "emotion:" << episode.relevance.emotional.emotion
           << ">";
        return ss.str();
    }

    bool Server::remove_by_uid(int uid) {
        QueryPtr query = _coll->createQuery();
        query->append("uid", uid);
        _coll->removeMessages(query);
        return true;
    }

    bool Server::episode_exists(int uid) {
        QueryPtr query = _coll->createQuery();
        query->append("uid", uid);
        try {
            _coll->findOne(query, true);
        }
        catch (const warehouse_ros::NoMatchingMessageException& exception) {
            return false;
        }
        return true;
    }

    MetadataPtr Server::makeMetadata(EpisodeCollectionPtr coll_ptr, const ltm::Episode& episode) {
//        // Create metadata, this data is used for queries
//        mongo_ros::Metadata metadata("x", grasp_vector.pose.position.x, "y", grasp_vector.pose.position.y,
//                                     "z", grasp_vector.pose.position.z);

        MetadataPtr meta = coll_ptr->createMetadata();
        meta->append("uid", (int) episode.uid);
        meta->append("type", episode.type);
        meta->append("source", episode.info.source);
        return meta;
    }

    // ==========================================================
    // ROS Services
    // ==========================================================

    bool Server::get_episode_service(ltm::GetEpisode::Request &req, ltm::GetEpisode::Response &res) {
        ROS_INFO_STREAM("GET: Retrieving episode with uid: " << req.uid);

        QueryPtr query = _coll->createQuery();
        query->append("uid", (int) req.uid);
        try {
            EpisodeWithMetadataPtr episode = _coll->findOne(query, false);
            res.episode = *episode;
        }
        catch (const warehouse_ros::NoMatchingMessageException& exception) {
            ROS_ERROR_STREAM("GET: Episode with uid '" << req.uid << "' not found.");
            res.succeeded = (uint8_t) false;
            return true;
        }
        res.succeeded = (uint8_t) true;
        return true;
    }

    bool Server::update_episode_service(ltm::UpdateEpisode::Request &req, ltm::UpdateEpisode::Response &res) {
        ROS_INFO_STREAM("Updating episode structure for uid: " << req.uid);
        res.succeeded = (uint8_t) true;
        return true;
    }

    bool Server::add_episode_service(ltm::AddEpisode::Request &req, ltm::AddEpisode::Response &res) {
        bool updating = false;
        if (episode_exists(req.episode.uid)) {
            if (!req.update) {
                ROS_ERROR_STREAM("ADD: Episode with uid '" << req.episode.uid << "' already exists.");
                res.succeeded = (uint8_t) false;
                return true;
            }
            updating = true;
            remove_by_uid(req.episode.uid);
        }
        _coll->insert(req.episode, makeMetadata(_coll, req.episode));
        ROS_INFO_STREAM_COND(updating, "ADD: Episode '" << req.episode.uid << "'updated. (" << _coll->count() << " entries)");
        ROS_INFO_STREAM_COND(!updating, "ADD: New episode '" << req.episode.uid << "'. (" << _coll->count() << " entries)");
        res.succeeded = (uint8_t) true;
        return true;
    }

    bool Server::status_service(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res) {
        show_status();
        return true;
    }

    bool Server::drop_db_service(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res) {
        // clear existing data
        ROS_INFO("DELETE: Dropping DB");
        _conn.dropDatabase(_db_name);
        show_status();
        return true;
    }
}


int main(int argc, char **argv) {
    // init node
    ros::init(argc, argv, "ltm_server");
    boost::scoped_ptr<ltm::Server> server(new ltm::Server());

    // run
    ros::spin();

    // close
    printf("\nClosing LTM server... \n\n");
    return 0;
}

