﻿cmake_minimum_required(VERSION 3.15)

set(CMAKE_AUTOMOC ON)

set(QWT_SOURCES
        qwt.h
        qwt_abstract_scale_draw.h
        qwt_array.h
        qwt_color_map.h
        qwt_clipper.h
        qwt_double_interval.h
        qwt_double_rect.h
        qwt_dyngrid_layout.h
        qwt_global.h
        qwt_layout_metrics.h
        qwt_math.h
        qwt_magnifier.h
        qwt_paint_buffer.h
        qwt_painter.h
        qwt_panner.h
        qwt_picker.h
        qwt_picker_machine.h
        qwt_polygon.h
        qwt_round_scale_draw.h
        qwt_scale_div.h
        qwt_scale_draw.h
        qwt_scale_engine.h
        qwt_scale_map.h
        qwt_spline.h
        qwt_symbol.h
        qwt_text_engine.h
        qwt_text_label.h
        qwt_text.h
        qwt_valuelist.h
        qwt_abstract_scale_draw.cpp
        qwt_color_map.cpp
        qwt_clipper.cpp
        qwt_double_interval.cpp
        qwt_double_rect.cpp
        qwt_dyngrid_layout.cpp
        qwt_layout_metrics.cpp
        qwt_math.cpp
        qwt_magnifier.cpp
        qwt_paint_buffer.cpp
        qwt_panner.cpp
        qwt_painter.cpp
        qwt_picker.cpp
        qwt_round_scale_draw.cpp
        qwt_scale_div.cpp
        qwt_scale_draw.cpp
        qwt_scale_map.cpp
        qwt_spline.cpp
        qwt_text_engine.cpp
        qwt_text_label.cpp
        qwt_text.cpp
        qwt_event_pattern.cpp
        qwt_picker_machine.cpp
        qwt_scale_engine.cpp
        qwt_symbol.cpp
        qwt_curve_fitter.h
        qwt_data.h
        qwt_event_pattern.h
        qwt_interval_data.h
        qwt_legend.h
        qwt_legend_item.h
        qwt_legend_itemmanager.h
        qwt_plot.h
        qwt_plot_curve.h
        qwt_plot_dict.h
        qwt_plot_grid.h
        qwt_plot_item.h
        qwt_plot_layout.h
        qwt_plot_marker.h
        qwt_plot_printfilter.h
        qwt_plot_rasteritem.h
        qwt_plot_spectrogram.h
        qwt_plot_scaleitem.h
        qwt_plot_canvas.h
        qwt_plot_rescaler.h
        qwt_plot_panner.h
        qwt_plot_picker.h
        qwt_plot_zoomer.h
        qwt_plot_magnifier.h
        qwt_raster_data.h
        qwt_scale_widget.h
        qwt_curve_fitter.cpp
        qwt_data.cpp
        qwt_interval_data.cpp
        qwt_legend.cpp
        qwt_legend_item.cpp
        qwt_plot.cpp
        qwt_plot_print.cpp
        qwt_plot_xml.cpp
        qwt_plot_axis.cpp
        qwt_plot_curve.cpp
        qwt_plot_dict.cpp
        qwt_plot_grid.cpp
        qwt_plot_item.cpp
        qwt_plot_spectrogram.cpp
        qwt_plot_scaleitem.cpp
        qwt_plot_marker.cpp
        qwt_plot_layout.cpp
        qwt_plot_printfilter.cpp
        qwt_plot_rasteritem.cpp
        qwt_plot_canvas.cpp
        qwt_plot_rescaler.cpp
        qwt_plot_panner.cpp
        qwt_plot_picker.cpp
        qwt_plot_zoomer.cpp
        qwt_plot_magnifier.cpp
        qwt_raster_data.cpp
        qwt_scale_widget.cpp
)

find_package(Qt5 COMPONENTS  Widgets  REQUIRED)

add_library(qwt)

target_compile_features(qwt PRIVATE cxx_std_17)

if(QWT_DISABLE_WARNINGS)
target_compile_options(qwt
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-w>
        $<$<CXX_COMPILER_ID:GNU>:-w>
        $<$<CXX_COMPILER_ID:MSVC>:/w>
)
endif()

target_include_directories(qwt INTERFACE . )

target_sources(qwt
    PRIVATE
        ${QWT_SOURCES}
)

target_link_libraries(qwt
    Qt5::Widgets
)
