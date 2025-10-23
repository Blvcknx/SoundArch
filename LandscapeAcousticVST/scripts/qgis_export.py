#!/usr/bin/env python3
"""
QGIS Script: Export DEM and Analysis Points for LandscapeAcousticVST

Usage:
1. Open DEM layer in QGIS (name it 'DEM')
2. Create point layer with 'source' and 'receiver' points (name it 'AnalysisPoints')
   - Add attribute field 'type' with values 'source' or 'receiver'
3. Run this script from QGIS Python Console
4. Exports JSON config file for plugin

Requirements:
- QGIS 3.x with PyQGIS
- DEM layer (raster)
- Point layer with type attribute
"""

from qgis.core import (QgsProject, QgsRasterLayer, QgsVectorLayer, 
                       QgsCoordinateTransform, QgsCoordinateReferenceSystem, 
                       QgsRectangle, QgsFeature, QgsGeometry, QgsPointXY)
from qgis.PyQt.QtWidgets import QFileDialog, QMessageBox
from qgis.PyQt.QtCore import QDate
import json
import os
from osgeo import gdal

def export_landscape_acoustic_config():
    """
    Export DEM cutout and point coordinates for acoustic modeling.
    
    Creates:
    1. Cropped DEM GeoTIFF around analysis area
    2. JSON configuration file with coordinates and parameters
    """
    
    try:
        # Get active project
        project = QgsProject.instance()
        
        # Find DEM layer
        dem_layer = None
        for layer in project.mapLayers().values():
            if isinstance(layer, QgsRasterLayer) and 'DEM' in layer.name().upper():
                dem_layer = layer
                break
        
        if not dem_layer:
            QMessageBox.warning(None, "Error", 
                              "No DEM layer found. Please add a raster layer named 'DEM'")
            return
        
        # Find analysis points layer  
        point_layer = None
        for layer in project.mapLayers().values():
            if isinstance(layer, QgsVectorLayer) and 'ANALYSIS' in layer.name().upper():
                point_layer = layer
                break
        
        if not point_layer:
            QMessageBox.warning(None, "Error", 
                              "No analysis points layer found. Please add a point layer named 'AnalysisPoints'")
            return
        
        # Extract source and receiver points
        source_point = None
        receiver_point = None
        
        for feature in point_layer.getFeatures():
            # Try different attribute names for point type
            point_type = None
            for field_name in ['type', 'Type', 'TYPE', 'point_type', 'category']:
                if field_name in [field.name() for field in feature.fields()]:
                    point_type = feature[field_name]
                    break
            
            if not point_type:
                continue
                
            geom = feature.geometry()
            if geom.type() != QgsWkbTypes.PointGeometry:
                continue
                
            coords = geom.asPoint()
            
            if point_type.lower() in ['source', 'src', 's']:
                source_point = {'lon': coords.x(), 'lat': coords.y()}
            elif point_type.lower() in ['receiver', 'recv', 'r', 'listener']:
                receiver_point = {'lon': coords.x(), 'lat': coords.y()}
        
        if not source_point or not receiver_point:
            QMessageBox.warning(None, "Error", 
                              "Both 'source' and 'receiver' points required in AnalysisPoints layer.\n" +
                              "Add 'type' attribute with values 'source' and 'receiver'")
            return
        
        # Calculate analysis extent (with buffer)
        buffer_deg = 0.005  # ~500m buffer at mid-latitudes
        
        min_lon = min(source_point['lon'], receiver_point['lon']) - buffer_deg
        max_lon = max(source_point['lon'], receiver_point['lon']) + buffer_deg
        min_lat = min(source_point['lat'], receiver_point['lat']) - buffer_deg
        max_lat = max(source_point['lat'], receiver_point['lat']) + buffer_deg
        
        extent = QgsRectangle(min_lon, min_lat, max_lon, max_lat)
        
        # Select output directory
        output_dir = QFileDialog.getExistingDirectory(None, "Select Output Directory")
        if not output_dir:
            return
        
        # Export cropped DEM
        dem_output = os.path.join(output_dir, 'exported_dem.tif')
        
        # Get DEM source path
        dem_source = dem_layer.source()
        
        # Crop DEM using GDAL
        try:
            # Use GDAL Warp to crop and reproject if needed
            warp_options = gdal.WarpOptions(
                format='GTiff',
                outputBounds=[extent.xMinimum(), extent.yMinimum(), 
                             extent.xMaximum(), extent.yMaximum()],
                dstSRS='EPSG:4326',  # Ensure WGS84 output
                xRes=None,  # Keep original resolution
                yRes=None,
                resampleAlg=gdal.GRA_Bilinear,
                creationOptions=['COMPRESS=LZW', 'TILED=YES']
            )
            
            gdal.Warp(dem_output, dem_source, options=warp_options)
            
        except Exception as e:
            QMessageBox.critical(None, "GDAL Error", f"Failed to export DEM: {str(e)}")
            return
        
        # Create configuration JSON
        config = {
            'format_version': '1.0',
            'export_info': {
                'date': QDate.currentDate().toString('yyyy-MM-dd'),
                'qgis_version': Qgis.QGIS_VERSION,
                'dem_layer': dem_layer.name(),
                'points_layer': point_layer.name()
            },
            'files': {
                'dem_file': os.path.abspath(dem_output),
                'dem_relative': 'exported_dem.tif'
            },
            'analysis_points': {
                'source': source_point,
                'receiver': receiver_point
            },
            'coordinate_system': {
                'crs': dem_layer.crs().authid(),
                'description': dem_layer.crs().description()
            },
            'extent': {
                'min_lon': extent.xMinimum(),
                'max_lon': extent.xMaximum(), 
                'min_lat': extent.yMinimum(),
                'max_lat': extent.yMaximum()
            },
            'default_parameters': {
                'temperature_C': 15.0,
                'humidity_percent': 70.0,
                'ground_type': 'porous',
                'source_height_m': 2.0,
                'receiver_height_m': 2.0
            }
        }
        
        # Export configuration
        config_output = os.path.join(output_dir, 'landscape_acoustic_config.json')
        with open(config_output, 'w', encoding='utf-8') as f:
            json.dump(config, f, indent=2, ensure_ascii=False)
        
        # Success message
        message = f"""✓ Export completed successfully!

Files created:
• DEM: {os.path.basename(dem_output)}
• Config: {os.path.basename(config_output)}

Analysis points:
• Source: {source_point['lon']:.6f}, {source_point['lat']:.6f}
• Receiver: {receiver_point['lon']:.6f}, {receiver_point['lat']:.6f}

Distance: {calculate_distance(source_point, receiver_point):.1f} m

Import the config file into LandscapeAcousticVST to continue analysis."""
        
        QMessageBox.information(None, "Export Successful", message)
        
    except Exception as e:
        QMessageBox.critical(None, "Export Error", f"An error occurred: {str(e)}")

def calculate_distance(point1, point2):
    """Calculate approximate distance between two geographic points."""
    import math
    
    # Haversine formula approximation
    R = 6371000  # Earth radius in meters
    
    lat1_rad = math.radians(point1['lat'])
    lat2_rad = math.radians(point2['lat'])
    dlat_rad = math.radians(point2['lat'] - point1['lat'])
    dlon_rad = math.radians(point2['lon'] - point1['lon'])
    
    a = (math.sin(dlat_rad/2)**2 + 
         math.cos(lat1_rad) * math.cos(lat2_rad) * math.sin(dlon_rad/2)**2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    
    return R * c

def create_example_points():
    """
    Create an example analysis points layer for testing.
    Call this function if you don't have analysis points yet.
    """
    
    # Create new point layer
    layer = QgsVectorLayer('Point?crs=EPSG:4326&field=type:string(20)', 
                          'AnalysisPoints', 'memory')
    
    if not layer.isValid():
        QMessageBox.critical(None, "Error", "Failed to create points layer")
        return
    
    # Add example points (adjust coordinates for your study area)
    provider = layer.dataProvider()
    
    # Example source point
    source_feature = QgsFeature()
    source_feature.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-105.5, 36.0)))
    source_feature.setAttributes(['source'])
    
    # Example receiver point  
    receiver_feature = QgsFeature()
    receiver_feature.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-105.4, 36.1)))
    receiver_feature.setAttributes(['receiver'])
    
    provider.addFeatures([source_feature, receiver_feature])
    layer.updateExtents()
    
    # Add layer to project
    QgsProject.instance().addMapLayer(layer)
    
    QMessageBox.information(None, "Points Created", 
                           "Example analysis points created. Adjust coordinates and run export.")

# Main execution
if __name__ == "__main__":
    export_landscape_acoustic_config()

# Alternative: Run this to create example points first
# create_example_points()

print("LandscapeAcousticVST export script loaded.")
print("Run: export_landscape_acoustic_config()")
print("Or run: create_example_points() to create test data")