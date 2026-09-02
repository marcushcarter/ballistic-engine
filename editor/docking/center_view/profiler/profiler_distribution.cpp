#include <editor/docking/center_view/profiler/profiler_distribution.h>
#include <drivers/imgui/imgui_helpers.h>
#include <core/rendering/renderer.h>
#include <imgui.h>
#include <implot.h>
#include <algorithm>
#include <cstdio>

namespace lumen {

void ProfilerDistribution::draw(EditorContext& ctx, const RenderGraphProfiler::Timing* selected, bool frozen)
{
    if (!selected) return;

    RenderGraphProfiler& prof = ctx.renderer->graph.profiler;

    if (selected->key != plot_key) { plot_key = selected->key; plot_hist.clear(); }
    if (!frozen) plot_hist.add(static_cast<float>(selected->raw_ms * 1000.0));

    imgui_title("%s · %s", selected->name, selected->type);

    const int n = plot_hist.data.Size;
    double avg = 0.0, median = 0.0, tmin = 0.0, tmax = 0.0;
    if (n > 0) {
        double sum = 0.0;
        tmin = tmax = plot_hist.data[0];
        for (int i = 0; i < n; ++i) {
            const float v = plot_hist.data[i];
            sum += v;
            tmin = std::min(tmin, (double)v);
            tmax = std::max(tmax, (double)v);
        }
        avg = sum / n;
        sort_scratch.assign(plot_hist.data.Data, plot_hist.data.Data + n);
        std::sort(sort_scratch.begin(), sort_scratch.end());
        median = (n & 1) ? sort_scratch[n / 2] : 0.5 * (sort_scratch[n / 2 - 1] + sort_scratch[n / 2]);
    }

    const std::vector<RenderGraphProfiler::Timing>& results = prof.results;
    int occ = 0;
    double total_us = 0.0;
    if (selected->parent != RenderGraphProfiler::INVALID && selected->parent < results.size()) {
        const char* sel_pass = results[selected->parent].name;
        for (const RenderGraphProfiler::Timing& t : results) {
            if (t.kind != RenderGraphProfiler::MarkKind::Draw) continue;
            if (t.parent == RenderGraphProfiler::INVALID || t.parent >= results.size()) continue;
            if (std::strcmp(results[t.parent].name, sel_pass) != 0) continue;
            if (std::strcmp(t.name, selected->name) != 0) continue;
            ++occ;
            total_us += t.gpu_ms * 1000.0;
        }
    }

    imgui_property_row_value_aligned("Time Avg", "%.0f µs", avg);
    imgui_property_row_value_aligned("Time Median", "%.0f µs", median);
    imgui_property_row_value_aligned("Time Min", "%.0f µs", tmin);
    imgui_property_row_value_aligned("Time Max", "%.0f µs", tmax);
    imgui_property_row_value_aligned("Total time this frame", "%.0f µs", total_us);
    imgui_property_row_value_aligned("Occurrences this frame", "%d", occ);
    imgui_spacing();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    const float width  = avail.x;
    const float height = width * 2.0f / 3.0f;
    const double y_max = (tmax > 0.0) ? tmax : 1.0;
    double ticks[2] = { 0.0, y_max };

    imgui_title("Time distribution");
    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0, 0, 0, 0));
    if (ImPlot::BeginPlot("##dist", ImVec2(width, height), ImPlotFlags_NoInputs | ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoTitle | ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, y_max, ImPlotCond_Always);
        ImPlot::PlotLine("µs", plot_hist.data.Data, plot_hist.data.Size, 1.0, 0.0);
        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

}