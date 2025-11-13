#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include <QPixmap>
#include <QImage>
#include <QFileDialog>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <cpl_conv.h>
#include <iostream>
#include <memory>

class ZoomableImageLabel : public QLabel {
    Q_OBJECT

public:
    explicit ZoomableImageLabel(QWidget *parent = nullptr) 
        : QLabel(parent), zoomFactor(1.0), originalPixmap() {
        setScaledContents(false);
        setAlignment(Qt::AlignCenter);
    }

    void setOriginalPixmap(const QPixmap &pixmap) {
        originalPixmap = pixmap;
        zoomFactor = 1.0;
        updateScaledPixmap();
    }

    void zoomIn() {
        zoomFactor *= 1.2;
        updateScaledPixmap();
    }

    void zoomOut() {
        zoomFactor /= 1.2;
        if (zoomFactor < 0.1) zoomFactor = 0.1;
        updateScaledPixmap();
    }

    void resetZoom() {
        zoomFactor = 1.0;
        updateScaledPixmap();
    }

    double getZoomFactor() const { return zoomFactor; }

protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    }

private:
    void updateScaledPixmap() {
        if (originalPixmap.isNull()) return;
        
        QSize newSize = originalPixmap.size() * zoomFactor;
        QPixmap scaled = originalPixmap.scaled(newSize, 
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setPixmap(scaled);
        resize(scaled.size());
    }

    double zoomFactor;
    QPixmap originalPixmap;
};

class DEMViewer : public QMainWindow {
    Q_OBJECT

public:
    DEMViewer(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Landscape Acoustic VST - DEM Viewer");
        resize(1024, 768);

        // Initialize GDAL
        GDALAllRegister();

        // Create menu bar
        createMenuBar();

        // Create central widget with scroll area
        imageLabel = new ZoomableImageLabel(this);
        
        scrollArea = new QScrollArea(this);
        scrollArea->setWidget(imageLabel);
        scrollArea->setWidgetResizable(false);
        scrollArea->setAlignment(Qt::AlignCenter);
        setCentralWidget(scrollArea);

        // Status bar
        statusBar()->showMessage("Ready. Open a DEM file to begin.");
    }

    ~DEMViewer() {
        // Cleanup is handled by Qt parent-child relationship
    }

private slots:
    void openDEM() {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "Open DEM File",
            QString(),
            "DEM Files (*.tif *.tiff *.asc *.hgt *.dt0 *.dt1 *.dt2);;All Files (*.*)"
        );

        if (fileName.isEmpty()) return;

        loadAndDisplayDEM(fileName);
    }

    void zoomIn() {
        imageLabel->zoomIn();
        updateStatusBar();
    }

    void zoomOut() {
        imageLabel->zoomOut();
        updateStatusBar();
    }

    void resetZoom() {
        imageLabel->resetZoom();
        updateStatusBar();
    }

    void about() {
        QMessageBox::about(this, "About DEM Viewer",
            "Landscape Acoustic VST - DEM Viewer\n\n"
            "A GDAL-based terrain visualization tool with hillshade rendering.\n\n"
            "Features:\n"
            "• Professional GDAL hillshade generation\n"
            "• Mouse wheel zoom\n"
            "• Pan with scroll bars\n"
            "• Support for GeoTIFF, ASCII Grid, DTED, SRTM formats\n\n"
            "Version 1.0.0 - November 2025");
    }

private:
    void createMenuBar() {
        QMenu *fileMenu = menuBar()->addMenu("&File");
        
        QAction *openAction = fileMenu->addAction("&Open DEM...");
        openAction->setShortcut(QKeySequence::Open);
        connect(openAction, &QAction::triggered, this, &DEMViewer::openDEM);

        fileMenu->addSeparator();

        QAction *exitAction = fileMenu->addAction("E&xit");
        exitAction->setShortcut(QKeySequence::Quit);
        connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

        QMenu *viewMenu = menuBar()->addMenu("&View");
        
        QAction *zoomInAction = viewMenu->addAction("Zoom &In");
        zoomInAction->setShortcut(QKeySequence::ZoomIn);
        connect(zoomInAction, &QAction::triggered, this, &DEMViewer::zoomIn);

        QAction *zoomOutAction = viewMenu->addAction("Zoom &Out");
        zoomOutAction->setShortcut(QKeySequence::ZoomOut);
        connect(zoomOutAction, &QAction::triggered, this, &DEMViewer::zoomOut);

        QAction *resetZoomAction = viewMenu->addAction("&Reset Zoom");
        resetZoomAction->setShortcut(Qt::CTRL + Qt::Key_0);
        connect(resetZoomAction, &QAction::triggered, this, &DEMViewer::resetZoom);

        QMenu *helpMenu = menuBar()->addMenu("&Help");
        
        QAction *aboutAction = helpMenu->addAction("&About");
        connect(aboutAction, &QAction::triggered, this, &DEMViewer::about);
    }

    void loadAndDisplayDEM(const QString &demPath) {
        statusBar()->showMessage("Loading DEM...");
        QApplication::processEvents();

        // Generate hillshade filename
        QFileInfo demInfo(demPath);
        QString hillshadePath = demInfo.absolutePath() + "/" + 
                               demInfo.baseName() + "_hillshade.tif";

        // Check if hillshade already exists
        QFileInfo hillshadeInfo(hillshadePath);
        if (!hillshadeInfo.exists()) {
            statusBar()->showMessage("Generating hillshade...");
            QApplication::processEvents();

            if (!generateHillshade(demPath, hillshadePath)) {
                QMessageBox::critical(this, "Error", 
                    "Failed to generate hillshade from DEM file.");
                statusBar()->showMessage("Failed to load DEM.");
                return;
            }
        }

        // Load hillshade as image
        statusBar()->showMessage("Loading hillshade image...");
        QApplication::processEvents();

        if (!loadHillshadeImage(hillshadePath)) {
            QMessageBox::critical(this, "Error", 
                "Failed to load hillshade image.");
            statusBar()->showMessage("Failed to display hillshade.");
            return;
        }

        statusBar()->showMessage(QString("Loaded: %1").arg(demInfo.fileName()));
        updateStatusBar();
    }

    bool generateHillshade(const QString &demPath, const QString &hillshadePath) {
        // Set GDAL options for hillshade generation
        const char *options[] = {
            "-z", "1.0",           // Z factor (vertical exaggeration)
            "-s", "1.0",           // Scale
            "-az", "315.0",        // Azimuth
            "-alt", "45.0",        // Altitude
            "-compute_edges",      // Compute values at edges
            nullptr
        };

        GDALDEMProcessingOptions *opts = GDALDEMProcessingOptionsNew(
            const_cast<char**>(options), nullptr);

        if (!opts) {
            std::cerr << "Failed to create GDAL processing options" << std::endl;
            return false;
        }

        // Open source DEM
        GDALDataset *srcDS = static_cast<GDALDataset*>(
            GDALOpen(demPath.toUtf8().constData(), GA_ReadOnly));

        if (!srcDS) {
            std::cerr << "Failed to open DEM file: " << demPath.toStdString() << std::endl;
            GDALDEMProcessingOptionsFree(opts);
            return false;
        }

        // Generate hillshade
        GDALDataset *hillshadeDS = static_cast<GDALDataset*>(
            GDALDEMProcessing(
                hillshadePath.toUtf8().constData(),
                srcDS,
                "hillshade",
                nullptr,
                opts,
                nullptr
            )
        );

        GDALClose(srcDS);
        GDALDEMProcessingOptionsFree(opts);

        if (!hillshadeDS) {
            std::cerr << "Failed to generate hillshade" << std::endl;
            return false;
        }

        GDALClose(hillshadeDS);
        return true;
    }

    bool loadHillshadeImage(const QString &hillshadePath) {
        // Open hillshade with GDAL
        GDALDataset *dataset = static_cast<GDALDataset*>(
            GDALOpen(hillshadePath.toUtf8().constData(), GA_ReadOnly));

        if (!dataset) {
            std::cerr << "Failed to open hillshade file" << std::endl;
            return false;
        }

        int width = dataset->GetRasterXSize();
        int height = dataset->GetRasterYSize();

        // Get first band
        GDALRasterBand *band = dataset->GetRasterBand(1);
        if (!band) {
            GDALClose(dataset);
            std::cerr << "No raster band found" << std::endl;
            return false;
        }

        // Check if image is too large
        const int maxDimension = 32768; // Reasonable limit for Qt
        if (width > maxDimension || height > maxDimension) {
            GDALClose(dataset);
            QMessageBox::warning(this, "Warning", 
                QString("DEM is very large (%1x%2). This may cause performance issues.\n"
                        "Consider downsampling the DEM first.").arg(width).arg(height));
            return false;
        }

        // Read hillshade data
        std::vector<unsigned char> data(width * height);
        CPLErr err = band->RasterIO(
            GF_Read, 0, 0, width, height,
            data.data(), width, height,
            GDT_Byte, 0, 0);

        GDALClose(dataset);

        if (err != CE_None) {
            std::cerr << "Failed to read raster data" << std::endl;
            return false;
        }

        // Convert to QImage
        QImage image(width, height, QImage::Format_Grayscale8);
        
        for (int y = 0; y < height; ++y) {
            unsigned char *scanLine = image.scanLine(y);
            for (int x = 0; x < width; ++x) {
                scanLine[x] = data[y * width + x];
            }
        }

        // Create colored hillshade for better visualization
        QImage coloredImage(width, height, QImage::Format_RGB888);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                unsigned char value = image.pixel(x, y) & 0xFF;
                
                // Apply color gradient (brown-green-white)
                int r, g, b;
                if (value < 128) {
                    // Brown to green
                    double t = value / 128.0;
                    r = static_cast<int>(139 * (1 - t) + 100 * t);
                    g = static_cast<int>(90 * (1 - t) + 150 * t);
                    b = static_cast<int>(43 * (1 - t) + 80 * t);
                } else {
                    // Green to white
                    double t = (value - 128) / 127.0;
                    r = static_cast<int>(100 * (1 - t) + 255 * t);
                    g = static_cast<int>(150 * (1 - t) + 255 * t);
                    b = static_cast<int>(80 * (1 - t) + 255 * t);
                }
                
                coloredImage.setPixel(x, y, qRgb(r, g, b));
            }
        }

        // Convert to pixmap and display
        QPixmap pixmap = QPixmap::fromImage(coloredImage);
        imageLabel->setOriginalPixmap(pixmap);

        return true;
    }

    void updateStatusBar() {
        double zoom = imageLabel->getZoomFactor();
        statusBar()->showMessage(
            QString("Zoom: %1%").arg(static_cast<int>(zoom * 100)));
    }

    ZoomableImageLabel *imageLabel;
    QScrollArea *scrollArea;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    DEMViewer viewer;
    viewer.show();
    
    return app.exec();
}
