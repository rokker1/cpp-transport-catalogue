#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>

class Serializer {

public:
    Serializer() = delete;
    Serializer(const catalogue::TransportCatalogue& catalogue,
                const catalogue::RoutingSettings& routing_settings,
                const renderer::RenderSettings& render_settings)
        : catalogue_(catalogue)
        , routing_settings_(routing_settings)
        , render_settings_(render_settings)
        
    {
        // Заполнить pb_catalogue_
        tc_pb::transport_catalogue pb_catalogue_;
        // остановки
        for (const auto& stop : catalogue_.GetStops()) {
            tc_pb::Stop pb_stop;
            pb_stop.set_id(stop.id);
            pb_stop.set_name(stop.name_);
            pb_stop.mutable_coordinates()->set_lat(stop.cordinates_.lat);
            pb_stop.mutable_coordinates()->set_lng(stop.cordinates_.lng);
            pb_catalogue_.mutable_stops()->Add(std::move(pb_stop));
        }

        //автобусы
        for(const auto& bus : catalogue_.GetBuses()) {
            tc_pb::Bus pb_bus;
            pb_bus.set_id(bus.id);
            pb_bus.set_name(bus.name_);
            
            for(const auto& stop : bus.stops_) {
                pb_bus.add_stops(stop->id);
            }

            switch (bus.bus_type_)
            {
            case BusType::ORDINARY:
                pb_bus.set_bus_type_cycled(false);
                break;
            case BusType::CYCLED:
                pb_bus.set_bus_type_cycled(true);
                break;
            default:
                break;
            }
            pb_catalogue_.mutable_buses()->Add(std::move(pb_bus));
        }

        // stops_to_buses_
        for(const auto& stop_to_buses : catalogue_.GetStopsToBuses()) {
            tc_pb::StopToBuses pb_stop_to_buses;                
            
            int stop_id = stop_to_buses.first->id;
            pb_stop_to_buses.set_stop_id(stop_id);

            const std::set<const Bus*>& buses = stop_to_buses.second;
            for(const auto& bus : buses) {
                pb_stop_to_buses.mutable_bus_id()->Add(bus->id);
            }
            pb_catalogue_.mutable_stops_to_buses()->Add(std::move(pb_stop_to_buses));
        }

        //intervals_to_distance_
        for(const auto& interval_to_distance : catalogue_.GetIntervalsToDistance()) {
            tc_pb::IntervalToDistance pb_interval_to_distance;
            int from_id = interval_to_distance.first.first->id;
            int to_id = interval_to_distance.first.second->id;
            int64_t distance = interval_to_distance.second;
            pb_interval_to_distance.set_from_id(from_id);
            pb_interval_to_distance.set_to_id(to_id);
            pb_interval_to_distance.set_distance(distance);
            pb_catalogue_.mutable_intervals_to_distance()->Add(std::move(pb_interval_to_distance));
        }

        *pb_base_.mutable_cat() = std::move(pb_catalogue_);

        // Заполнить routing_settings
        FillRoutingSettings();

        FillRenderSettings();
    }

    void SaveTo(const std::filesystem::path& path) const {
        std::fstream out(path, std::ios::binary);

        pb_base_.SerializePartialToOstream(&out);
    }
    
private:
    const catalogue::TransportCatalogue& catalogue_;
    const catalogue::RoutingSettings& routing_settings_;
    const renderer::RenderSettings& render_settings_;
    tc_pb::TransportBase pb_base_;

    struct RenderSettingsColorVisitor {
        tc_pb::Color& pb_color;

        void operator()(std::monostate) {
            pb_color.set_is_initialized(false);
        }
        void operator()(const std::string& color) {
            *pb_color.mutable_string_color() = color;
            pb_color.set_is_initialized(true);
        }
        void operator()(svg::Rgb rgb) {
            pb_color.mutable_rgb_color()->set_red(rgb.red);
            pb_color.mutable_rgb_color()->set_green(rgb.green);
            pb_color.mutable_rgb_color()->set_blue(rgb.blue);
            pb_color.set_is_initialized(true);
        } 
        void operator()(svg::Rgba rgba) {
            pb_color.mutable_rgba_color()->set_red(rgba.red);
            pb_color.mutable_rgba_color()->set_green(rgba.green);
            pb_color.mutable_rgba_color()->set_blue(rgba.blue);
            pb_color.mutable_rgba_color()->set_opacity(rgba.opacity);
            pb_color.set_is_initialized(true);
        } 
    };

    void FillRoutingSettings() {
        tc_pb::RoutingSettings pb_routing_settings_;

        pb_routing_settings_.set_bus_velocity(routing_settings_.bus_velocity);
        pb_routing_settings_.set_bus_wait_time(routing_settings_.bus_wait_time);

        *pb_base_.mutable_routing_settings() = std::move(pb_routing_settings_);
    }

    void FillRenderSettings() {
        tc_pb::RenderSettings pb_render_settings;
        pb_render_settings.set_width(render_settings_.width);
        pb_render_settings.set_height(render_settings_.height);
        pb_render_settings.set_padding(render_settings_.padding);
        pb_render_settings.set_line_width(render_settings_.line_width);
        pb_render_settings.set_stop_radius(render_settings_.stop_radius);

        pb_render_settings.set_bus_label_font_size(render_settings_.bus_label_font_size);
        pb_render_settings.mutable_bus_label_offset()->set_x(render_settings_.bus_label_offset.x);
        pb_render_settings.mutable_bus_label_offset()->set_y(render_settings_.bus_label_offset.y);

        pb_render_settings.set_stop_label_font_size(render_settings_.stop_label_font_size);
        pb_render_settings.mutable_stop_label_offset()->set_x(render_settings_.stop_label_offset.x);
        pb_render_settings.mutable_stop_label_offset()->set_y(render_settings_.stop_label_offset.y);

        tc_pb::Color pb_color;
        visit(RenderSettingsColorVisitor{pb_color}, render_settings_.underlayer_color);

        pb_render_settings.set_underlayer_width(render_settings_.underlayer_width);

        // visit vector<Color> ???
        for(const auto& color : render_settings_.color_palette) {
            tc_pb::Color pb_color;
            visit(RenderSettingsColorVisitor{pb_color}, color);
            pb_render_settings.mutable_color_palette()->Add(std::move(pb_color));
        }

        *pb_base_.mutable_rendder_settings() = std::move(pb_render_settings);
    }



};