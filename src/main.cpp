#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <iomanip>
#include "RandomDatasetGenerator.h"
#include <SFML/Graphics.hpp>

//fastest single-threaded map candidates for all int and float types
#include "Radix_Flat_Map.h" //fast with read-heavy workloads
#include "Batch_N_Hash_List.h" //fast with write-heavy workloads or batch lookup only
#include <map> //best for abstract data types
#include "X-fast_Trie.h" //best for mixed workloads
#include "AVL_Tree.h" //could be better for abstract data types
#include "Treap.h" //lowest constant factors

sf::VertexArray createPlot(const std::vector<sf::Vector2f>& points, float max_x, float max_y,
						   const sf::Color& color, float width, float height, float padding) {
	sf::VertexArray plot(sf::LinesStrip);
	float plotWidth = width - 2 * padding;
	float plotHeight = height - 2 * padding;

	for (const auto& p : points) {
		// Scale and shift coordinaates to fit the plot area
		float x = padding + (p.x / max_x) * plotWidth;
		float y = padding + plotHeight - (p.y / max_y) * plotHeight; // Y is inverted
		plot.append(sf::Vertex(sf::Vector2f(x, y), color));
	}
	return plot;
}

int main(){
	// Performance graph (insertion - size_t)
	constexpr size_t Xpoint_MAX = 40000;
	constexpr size_t STRIDE = 1000;
	std::vector<sf::Vector2f> stl_points;
	std::vector<sf::Vector2f> rf_points;
	std::vector<sf::Vector2f> rf_batch_points;
	std::vector<sf::Vector2f> bnhl_points;
	std::vector<sf::Vector2f> xft_points;
	std::vector<sf::Vector2f> avl_points;
	std::vector<sf::Vector2f> treap_points;

	double max_elapsed = 0.000001; // Avoid division by zero, start with a tiny value

	for(size_t i = 1; i <= Xpoint_MAX; i += STRIDE) {
		// Init maps
		std::map<size_t,int> stl_map;
		Radix_Flat_Map<size_t,int> rf_map;
		rf_map.reserve(i);
		Radix_Flat_Map<size_t,int> rf_batch_map;  // NEW: for batch insertion
		rf_batch_map.reserve(i);
		Batch_N_Hash_List<size_t,int> bnhl(i);
		XFastTrie<size_t,int> xft(i);
		AVL_Tree<size_t,int> hash_avl_tree(i);
		Treap<size_t,int> treap;

		// Init dataset
		RandomDatasetGenerator rand_dataset(i);

		// standard library map (red)
		auto start = std::chrono::high_resolution_clock::now();
		for(size_t j = 0; j < i; j++) {
			stl_map.emplace(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
		}
		auto elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
		stl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
		if (elapsed > max_elapsed) max_elapsed = elapsed;

		// radix flat map (green)
		start = std::chrono::high_resolution_clock::now();
		for(size_t j = 0; j < i; j++) {
			rf_map.insert(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
		}
		elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
		rf_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
		if (elapsed > max_elapsed) max_elapsed = elapsed;

		// radix flat map BATCH insertion (orange)
		start = std::chrono::high_resolution_clock::now();
		// Create pairs for batch insertion
		std::vector<std::pair<size_t, int>> batch_data;
		batch_data.reserve(i);
		for(size_t j = 0; j < i; j++) {
			batch_data.emplace_back(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
		}
		rf_batch_map.insert_batch(batch_data.begin(), batch_data.end());
		elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
		rf_batch_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
		if (elapsed > max_elapsed) max_elapsed = elapsed;

		// batch N Hash list (cyan)
		start = std::chrono::high_resolution_clock::now();
		for(size_t j = 0; j < i; j++) {
			bnhl.addHead(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
		}
		elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
		bnhl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
		if (elapsed > max_elapsed) max_elapsed = elapsed;

		// X-fast trie (yellow)
		start = std::chrono::high_resolution_clock::now();
		for(size_t j = 0; j < i; j++) {
			xft.insert(rand_dataset.random_size_ts[j],rand_dataset.random_ints[j]);
		}
		elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
		xft_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
		if (elapsed > max_elapsed) max_elapsed = elapsed;

		// Hash map AVL tree (magenta)
		start = std::chrono::high_resolution_clock::now();
		for (size_t j = 0; j < i; j++) {
		hash_avl_tree.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
		}
		elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
		avl_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
		if (elapsed > max_elapsed) max_elapsed = elapsed;

		// Treap (pink)
		start = std::chrono::high_resolution_clock::now();
		for (size_t j = 0; j < i; j++) {
			treap.insert(rand_dataset.random_size_ts[j], rand_dataset.random_ints[j]);
		}
		elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
		treap_points.emplace_back(static_cast<float>(i), static_cast<float>(elapsed));
		if (elapsed > max_elapsed) max_elapsed = elapsed;
	}


	// --- 2. SFML Plotting ---
    constexpr float WIN_WIDTH = 1000.0f;
    constexpr float WIN_HEIGHT = 800.0f;
    constexpr float PADDING = 60.0f;

    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(WIN_WIDTH), static_cast<unsigned int>(WIN_HEIGHT)), "Map Insertion Benchmark");
    window.setFramerateLimit(60);

    // --- Create Plot Lines ---
    float max_x = static_cast<float>(Xpoint_MAX);
    float max_y = static_cast<float>(max_elapsed);

    sf::VertexArray plot_stl = createPlot(stl_points, max_x, max_y, sf::Color::Red, WIN_WIDTH, WIN_HEIGHT, PADDING);
    sf::VertexArray plot_rf = createPlot(rf_points, max_x, max_y, sf::Color::Green, WIN_WIDTH, WIN_HEIGHT, PADDING);
    sf::VertexArray plot_rf_batch = createPlot(rf_batch_points, max_x, max_y, sf::Color(255, 165, 0), WIN_WIDTH, WIN_HEIGHT, PADDING); // NEW
    sf::VertexArray plot_bnhl = createPlot(bnhl_points, max_x, max_y, sf::Color::Cyan, WIN_WIDTH, WIN_HEIGHT, PADDING);
    sf::VertexArray plot_xft = createPlot(xft_points, max_x, max_y, sf::Color::Yellow, WIN_WIDTH, WIN_HEIGHT, PADDING);
    sf::VertexArray plot_avl = createPlot(avl_points, max_x, max_y, sf::Color::Magenta, WIN_WIDTH, WIN_HEIGHT, PADDING);
    sf::VertexArray plot_treap = createPlot(treap_points, max_x, max_y, sf::Color(255, 192, 203), WIN_WIDTH, WIN_HEIGHT, PADDING); // Pink

    // --- Create Axes ---
    sf::VertexArray axes(sf::Lines);
    // Y-Axis
    axes.append(sf::Vertex(sf::Vector2f(PADDING, PADDING), sf::Color::White));
    axes.append(sf::Vertex(sf::Vector2f(PADDING, WIN_HEIGHT - PADDING), sf::Color::White));
    // X-Axis
    axes.append(sf::Vertex(sf::Vector2f(PADDING, WIN_HEIGHT - PADDING), sf::Color::White));
    axes.append(sf::Vertex(sf::Vector2f(WIN_WIDTH - PADDING, WIN_HEIGHT - PADDING), sf::Color::White));

    // --- Load Font --
    sf::Font font;
    bool font_loaded = false;

    // Try default font paths depending on OS
	#ifdef _WIN32
		font_loaded = font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
	#elif __APPLE__
		font_loaded = font.loadFromFile("/Library/Fonts/Arial.ttf");
	#else
		font_loaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
	#endif

	if (!font_loaded) {
		std::cerr << "Could not load a default font. Labels will be missing.\n";
	}

    // --- Create Labels ---
    sf::Text title, y_label, x_label, max_y_label, max_x_label;
    sf::Text legend_stl, legend_rf, legend_rf_batch, legend_bnhl, legend_xft, legend_avl, legend_treap;

    if (font_loaded) {
        title.setFont(font);
        title.setString("Times to insert N unsigned int (Max N = " + std::to_string(Xpoint_MAX) + ")");
        title.setCharacterSize(20);
        title.setPosition(WIN_WIDTH / 2.0f - title.getGlobalBounds().width / 2.0f, PADDING / 4.0f);

        y_label.setFont(font);
        y_label.setString("Time (s)");
        y_label.setCharacterSize(16);
        y_label.setRotation(-90);
        y_label.setPosition(PADDING / 2.5f, WIN_HEIGHT / 2.0f + y_label.getGlobalBounds().width / 2.0f);

        x_label.setFont(font);
        x_label.setString("N (unsigned int)");
        x_label.setCharacterSize(16);
        x_label.setPosition(WIN_WIDTH / 2.0f - x_label.getGlobalBounds().width / 2.0f, WIN_HEIGHT - PADDING / 1.5f);

        // Format max_y to be readable
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << max_y;
        max_y_label.setFont(font);
        max_y_label.setString(ss.str() + "s");
        max_y_label.setCharacterSize(12);
        max_y_label.setPosition(PADDING + 5, PADDING - 5);

        max_x_label.setFont(font);
        max_x_label.setString(std::to_string(Xpoint_MAX));
        max_x_label.setCharacterSize(12);
        max_x_label.setPosition(WIN_WIDTH - PADDING - max_x_label.getGlobalBounds().width - 5, WIN_HEIGHT - PADDING + 5);

        // Legend
        float legend_x = PADDING + 20.0f;
        float legend_y = PADDING + 20.0f;

        legend_stl.setFont(font);
        legend_stl.setString("Red: std::map");
        legend_stl.setCharacterSize(12);
        legend_stl.setPosition(legend_x, legend_y);
        legend_stl.setFillColor(sf::Color::Red);

        legend_rf.setFont(font);
        legend_rf.setString("Green: Radix_Flat_Map");
        legend_rf.setCharacterSize(12);
        legend_rf.setPosition(legend_x, legend_y + 20);
        legend_rf.setFillColor(sf::Color::Green);

        legend_rf_batch.setFont(font);
        legend_rf_batch.setString("Orange: Radix_Flat_Map (Batch)");
        legend_rf_batch.setCharacterSize(12);
        legend_rf_batch.setPosition(legend_x, legend_y + 40);
        legend_rf_batch.setFillColor(sf::Color(255, 165, 0));

        legend_bnhl.setFont(font);
        legend_bnhl.setString("Cyan: Batch_N_Hash_List");
        legend_bnhl.setCharacterSize(12);
        legend_bnhl.setPosition(legend_x, legend_y + 60);  // Shifted down
        legend_bnhl.setFillColor(sf::Color::Cyan);

        legend_xft.setFont(font);
        legend_xft.setString("Yellow: XFastTrie");
        legend_xft.setCharacterSize(12);
        legend_xft.setPosition(legend_x, legend_y + 80);  // Shifted down
        legend_xft.setFillColor(sf::Color::Yellow);

        legend_avl.setFont(font);
        legend_avl.setString("Magenta: Hash_Map_AVL_Tree");
        legend_avl.setCharacterSize(12);
        legend_avl.setPosition(legend_x, legend_y + 100);  // Shifted down
        legend_avl.setFillColor(sf::Color::Magenta);

        legend_treap.setFont(font);
        legend_treap.setString("Pink: Treap");
        legend_treap.setCharacterSize(12);
        legend_treap.setPosition(legend_x, legend_y + 120);  // Shifted down
        legend_treap.setFillColor(sf::Color(255, 192, 203));
    }

    // --- 3. Main SFML Loop ---
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color(10, 10, 30)); // Dark blue background

        // Draw plots
        window.draw(plot_stl);
        window.draw(plot_rf);
        window.draw(plot_rf_batch);  // NEW
        window.draw(plot_bnhl);
        window.draw(plot_xft);
        window.draw(plot_avl);
        window.draw(plot_treap);

        // Draw UI
        window.draw(axes);
        if (font_loaded) {
            window.draw(title);
            window.draw(x_label);
            window.draw(y_label);
            window.draw(max_x_label);
            window.draw(max_y_label);
            window.draw(legend_stl);
            window.draw(legend_rf);
            window.draw(legend_rf_batch);  // NEW
            window.draw(legend_bnhl);
            window.draw(legend_xft);
            window.draw(legend_avl);
            window.draw(legend_treap);
        }

        window.display();
    }

    return 0;
}