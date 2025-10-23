"""
LandscapeAcousticVST QGIS Plugin
Exports DEM data and analysis points for acoustic modeling
"""

from qgis.PyQt.QtWidgets import QAction, QMessageBox
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QSettings, QTranslator, QCoreApplication
from qgis.core import QgsApplication
import os.path
import sys

class LandscapeAcousticPlugin:
    """QGIS Plugin Implementation."""

    def __init__(self, iface):
        """Constructor.
        
        :param iface: An interface instance that will be passed to this class
            which provides the hook by which you can manipulate the QGIS
            application at run time.
        :type iface: QgsInterface
        """
        self.iface = iface
        self.plugin_dir = os.path.dirname(__file__)
        
        # Initialize locale
        locale = QSettings().value('locale/userLocale')[0:2]
        locale_path = os.path.join(
            self.plugin_dir,
            'i18n',
            'landscape_acoustic_{}.qm'.format(locale))

        if os.path.exists(locale_path):
            self.translator = QTranslator()
            self.translator.load(locale_path)
            QCoreApplication.installTranslator(self.translator)

        # Declare instance attributes
        self.actions = []
        self.menu = self.tr(u'&Landscape Acoustic Exporter')

    def tr(self, message):
        """Get the translation for a string using Qt translation API."""
        return QCoreApplication.translate('LandscapeAcousticPlugin', message)

    def add_action(
        self,
        icon_path,
        text,
        callback,
        enabled_flag=True,
        add_to_menu=True,
        add_to_toolbar=True,
        status_tip=None,
        whats_this=None,
        parent=None):
        """Add a toolbar icon to the toolbar."""

        icon = QIcon(icon_path)
        action = QAction(icon, text, parent)
        action.triggered.connect(callback)
        action.setEnabled(enabled_flag)

        if status_tip is not None:
            action.setStatusTip(status_tip)

        if whats_this is not None:
            action.setWhatsThis(whats_this)

        if add_to_toolbar:
            # Adds plugin icon to Plugins toolbar
            self.iface.addToolBarIcon(action)

        if add_to_menu:
            self.iface.addPluginToMenu(
                self.menu,
                action)

        self.actions.append(action)
        return action

    def initGui(self):
        """Create the menu entries and toolbar icons inside the QGIS GUI."""

        icon_path = os.path.join(self.plugin_dir, 'icon.png')
        self.add_action(
            icon_path,
            text=self.tr(u'Export for LandscapeAcousticVST'),
            callback=self.run,
            parent=self.iface.mainWindow())

    def unload(self):
        """Removes the plugin menu item and icon from QGIS GUI."""
        for action in self.actions:
            self.iface.removePluginMenu(
                self.tr(u'&Landscape Acoustic Exporter'),
                action)
            self.iface.removeToolBarIcon(action)

    def run(self):
        """Run method that performs all the real work"""
        
        try:
            # Import the export function
            from . import qgis_export
            qgis_export.export_landscape_acoustic_config()
            
        except ImportError as e:
            QMessageBox.critical(
                self.iface.mainWindow(),
                "Import Error", 
                f"Failed to import export module: {str(e)}")
        except Exception as e:
            QMessageBox.critical(
                self.iface.mainWindow(),
                "Export Error",
                f"Export failed: {str(e)}")


def classFactory(iface):
    """Load LandscapeAcousticPlugin class from file landscape_acoustic.
    
    :param iface: A QGIS interface instance.
    :type iface: QgsInterface
    """
    return LandscapeAcousticPlugin(iface)