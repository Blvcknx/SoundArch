"""
LandscapeAcousticVST QGIS Plugin Init
"""

def classFactory(iface):
    from .landscape_acoustic import LandscapeAcousticPlugin
    return LandscapeAcousticPlugin(iface)