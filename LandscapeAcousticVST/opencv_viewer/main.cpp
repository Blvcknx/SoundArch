#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>

class HillshadeViewer {
private:
    cv::Mat original_image_;
    cv::Mat display_image_;
    std::string window_name_;
    double zoom_factor_;
    cv::Point pan_offset_;
    bool dragging_;
    cv::Point last_mouse_pos_;

    void updateDisplay() {
        if (original_image_.empty()) return;

        // Calculate visible region
        int start_x = std::max(0, -pan_offset_.x);
        int start_y = std::max(0, -pan_offset_.y);
        int end_x = std::min(original_image_.cols, static_cast<int>((cv::getWindowImageRect(window_name_).width / zoom_factor_) - pan_offset_.x));
        int end_y = std::min(original_image_.rows, static_cast<int>((cv::getWindowImageRect(window_name_).height / zoom_factor_) - pan_offset_.y));

        if (end_x <= start_x || end_y <= start_y) return;

        cv::Rect roi(start_x, start_y, end_x - start_x, end_y - start_y);
        cv::Mat visible_region = original_image_(roi);

        // Resize for display
        cv::resize(visible_region, display_image_, cv::Size(), zoom_factor_, zoom_factor_, cv::INTER_LINEAR);
    }

    static void onMouse(int event, int x, int y, int flags, void* userdata) {
        HillshadeViewer* viewer = static_cast<HillshadeViewer*>(userdata);
        viewer->handleMouse(event, x, y, flags);
    }

    void handleMouse(int event, int x, int y, int flags) {
        switch (event) {
            case cv::EVENT_LBUTTONDOWN:
                dragging_ = true;
                last_mouse_pos_ = cv::Point(x, y);
                break;
            case cv::EVENT_LBUTTONUP:
                dragging_ = false;
                break;
            case cv::EVENT_MOUSEMOVE:
                if (dragging_) {
                    cv::Point delta = cv::Point(x, y) - last_mouse_pos_;
                    pan_offset_.x += static_cast<int>(delta.x / zoom_factor_);
                    pan_offset_.y += static_cast<int>(delta.y / zoom_factor_);
                    last_mouse_pos_ = cv::Point(x, y);
                    updateDisplay();
                    cv::imshow(window_name_, display_image_);
                }
                break;
            case cv::EVENT_MOUSEWHEEL:
                double zoom_delta = (flags > 0) ? 1.1 : 0.9;
                double new_zoom = zoom_factor_ * zoom_delta;

                // Limit zoom range
                if (new_zoom >= 0.1 && new_zoom <= 10.0) {
                    // Zoom towards mouse position
                    cv::Point mouse_pos(x, y);
                    cv::Point image_pos = (mouse_pos / zoom_factor_) - pan_offset_;

                    zoom_factor_ = new_zoom;
                    pan_offset_ = mouse_pos / zoom_factor_ - image_pos;

                    updateDisplay();
                    cv::imshow(window_name_, display_image_);
                }
                break;
        }
    }

public:
    HillshadeViewer() : zoom_factor_(1.0), dragging_(false), pan_offset_(0, 0) {
        window_name_ = "DEM Hillshade Viewer";
    }

    bool loadImage(const std::string& filename) {
        // Try to load as grayscale first
        original_image_ = cv::imread(filename, cv::IMREAD_UNCHANGED);

        if (original_image_.empty()) {
            std::cout << "Failed to load image: " << filename << std::endl;
            return false;
        }

        // Convert to 8-bit grayscale if needed
        if (original_image_.type() != CV_8UC1) {
            if (original_image_.channels() > 1) {
                cv::cvtColor(original_image_, original_image_, cv::COLOR_BGR2GRAY);
            }

            if (original_image_.type() != CV_8UC1) {
                double minVal, maxVal;
                cv::minMaxLoc(original_image_, &minVal, &maxVal);
                if (maxVal > minVal) {
                    original_image_.convertTo(original_image_, CV_8UC1, 255.0 / (maxVal - minVal),
                                            -minVal * 255.0 / (maxVal - minVal));
                } else {
                    original_image_.convertTo(original_image_, CV_8UC1);
                }
            }
        }

        std::cout << "Loaded image: " << filename << std::endl;
        std::cout << "Image size: " << original_image_.cols << "x" << original_image_.rows << std::endl;
        std::cout << "Image type: " << original_image_.type() << std::endl;

        updateDisplay();
        return true;
    }

    void show() {
        if (original_image_.empty()) {
            std::cout << "No image loaded!" << std::endl;
            return;
        }

        cv::namedWindow(window_name_, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO | cv::WINDOW_GUI_EXPANDED);
        cv::setMouseCallback(window_name_, onMouse, this);

        // Set initial window size to fit screen
        cv::resizeWindow(window_name_, 1200, 800);

        cv::imshow(window_name_, display_image_);

        std::cout << "\nControls:" << std::endl;
        std::cout << "  Mouse wheel: Zoom in/out" << std::endl;
        std::cout << "  Left click + drag: Pan" << std::endl;
        std::cout << "  ESC or 'q': Quit" << std::endl;
        std::cout << "  'r': Reset view" << std::endl;
        std::cout << "  'f': Fit to window" << std::endl;

        while (true) {
            int key = cv::waitKey(10);
            if (key == 27 || key == 'q' || key == 'Q') { // ESC or 'q'
                break;
            } else if (key == 'r' || key == 'R') { // Reset view
                zoom_factor_ = 1.0;
                pan_offset_ = cv::Point(0, 0);
                updateDisplay();
                cv::imshow(window_name_, display_image_);
            } else if (key == 'f' || key == 'F') { // Fit to window
                cv::Rect window_rect = cv::getWindowImageRect(window_name_);
                double scale_x = static_cast<double>(window_rect.width) / original_image_.cols;
                double scale_y = static_cast<double>(window_rect.height) / original_image_.rows;
                zoom_factor_ = std::min(scale_x, scale_y);
                pan_offset_ = cv::Point(0, 0);
                updateDisplay();
                cv::imshow(window_name_, display_image_);
            }
        }

        cv::destroyWindow(window_name_);
    }

    void resetView() {
        zoom_factor_ = 1.0;
        pan_offset_ = cv::Point(0, 0);
        updateDisplay();
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: opencv_hillshade_viewer <hillshade_image.tif/png>" << std::endl;
        std::cout << std::endl;
        std::cout << "To generate hillshade from DEM first:" << std::endl;
        std::cout << "  gdaldem hillshade input_dem.tif output_hillshade.tif" << std::endl;
        std::cout << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  Mouse wheel: Zoom in/out" << std::endl;
        std::cout << "  Left click + drag: Pan around" << std::endl;
        std::cout << "  'r': Reset view to original" << std::endl;
        std::cout << "  'f': Fit image to window" << std::endl;
        std::cout << "  ESC or 'q': Quit" << std::endl;
        return -1;
    }

    HillshadeViewer viewer;

    if (!viewer.loadImage(argv[1])) {
        return -1;
    }

    viewer.show();
    return 0;
}