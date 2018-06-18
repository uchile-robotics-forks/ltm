#include <ltm/db/episode_metadata.h>


namespace ltm {
    namespace db {

        void EpisodeMetadataBuilder::make_meta_episode(const Episode &node, MetadataPtr meta) {
            meta->append("uid", (int) node.uid);
            meta->append("type", node.type);
            meta->append("parent_id", (int) node.parent_id);
            meta->append("children_ids", node.children_ids);
            meta->append("tags", node.tags);
            meta->append("children_tags", node.children_tags);
        }

        void EpisodeMetadataBuilder::make_meta_info(const Info &node, MetadataPtr meta) {
            meta->append("info.source", node.source);
        }

        void EpisodeMetadataBuilder::make_meta_when(const When &node, MetadataPtr meta) {
            double start = node.start.sec + node.start.nsec * pow10(-9);
            double end = node.end.sec + node.end.nsec * pow10(-9);
            meta->append("when.start", start);
            meta->append("when.end", end);
        }

        void EpisodeMetadataBuilder::make_meta_where(const Where &node, MetadataPtr meta) {
            meta->append("where.frame_id", node.frame_id);
            meta->append("where.map_name", node.map_name);
            meta->append("where.position_x", node.position.x);
            meta->append("where.position_y", node.position.y);
            meta->append("where.location", node.location);
            meta->append("where.area", node.area);
            meta->append("where.children_locations", node.children_locations);
            meta->append("where.children_areas", node.children_areas);
        }

        void EpisodeMetadataBuilder::make_meta_what(const What &node, MetadataPtr meta) {
            // STREAMS
            std::vector<std::string> stream_types;
            std::vector<uint32_t> stream_uids;
            std::vector<ltm::StreamRegister>::const_iterator s_it;
            for (s_it = node.streams.begin(); s_it != node.streams.end(); ++s_it) {
                stream_types.push_back(s_it->type);
                stream_uids.push_back(s_it->uid);
            }
            meta->append("what.stream.types", stream_types);
            meta->append("what.stream.uids", stream_uids);

            // ENTITIES
            std::vector<std::string> entity_types;
            std::vector<uint32_t> entity_uids;
            std::vector<ltm::EntityRegister>::const_iterator e_it;
            for (e_it = node.entities.begin(); e_it != node.entities.end(); ++e_it) {
                entity_types.push_back(e_it->type);
                entity_uids.push_back(e_it->uid);
            }
            meta->append("what.entities.types", entity_types);
            meta->append("what.entities.uids", entity_uids);
        }

        void EpisodeMetadataBuilder::make_meta_relevance(const Relevance &node, MetadataPtr meta) {
            make_meta_relevance_emotional(node.emotional, meta);
            make_meta_relevance_historical(node.historical, meta);
        }

        void EpisodeMetadataBuilder::make_meta_relevance_historical(const HistoricalRelevance &node, MetadataPtr meta) {
            meta->append("relevance.historical.value", node.value);

            char buff[16];
            snprintf(buff, sizeof(buff), "%04u/%02u/%02u", node.last_update.year, node.last_update.month,
                     node.last_update.day);
            std::string last_update = buff;
            snprintf(buff, sizeof(buff), "%04u/%02u/%02u", node.next_update.year, node.next_update.month,
                     node.next_update.day);
            std::string next_update = buff;
            meta->append("relevance.historical.last_update", last_update);
            meta->append("relevance.historical.next_update", next_update);
        }

        void EpisodeMetadataBuilder::make_meta_relevance_emotional(const EmotionalRelevance &node, MetadataPtr meta) {
            meta->append("relevance.emotional.emotion", node.emotion);
            meta->append("relevance.emotional.value", node.value);
            meta->append("relevance.emotional.children_emotions", node.children_emotions);
            meta->append("relevance.emotional.children_values", node.children_values);
        }

        MetadataPtr EpisodeMetadataBuilder::make_metadata(const Episode &episode, EpisodeCollectionPtr &coll) {
            MetadataPtr meta = coll->createMetadata();
            make_meta_episode(episode, meta);
            make_meta_info(episode.info, meta);
            make_meta_when(episode.when, meta);
            make_meta_where(episode.where, meta);
            make_meta_what(episode.what, meta);
            make_meta_relevance(episode.relevance, meta);
            return meta;
        }
        
    }
}