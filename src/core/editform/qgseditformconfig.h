/***************************************************************************
                          qgseditformconfig.h
                             -------------------
    begin                : Nov 18, 2015
    copyright            : (C) 2015 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEDITFORMCONFIG_H
#define QGSEDITFORMCONFIG_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QMap>
#include <QDomElement>
#include <QDomDocument>

#include "qgsattributeeditorelement.h"
#include "qgsreadwritecontext.h"

class QgsRelationManager;
class QgsEditFormConfigPrivate;
class QgsAttributeEditorContainer;

/**
 * \ingroup core
 * \class QgsEditFormConfig
 * \brief Contains configuration settings for an editor form.
 */
class CORE_EXPORT QgsEditFormConfig
{

    Q_GADGET

  public:

    //! The different types to layout the attribute editor.
    enum EditorLayout
    {
      GeneratedLayout = 0, //!< Autogenerate a simple tabular layout for the form
      TabLayout = 1,       //!< Use a layout with tabs and group boxes. Needs to be configured.
      UiFileLayout = 2     //!< Load a .ui file for the layout. Needs to be configured.
    };
    Q_ENUM( EditorLayout )

    struct GroupData
    {
      //! Constructor for GroupData
      GroupData() = default;
      GroupData( const QString &name, const QList<QString> &fields )
        : mName( name )
        , mFields( fields )
      {}
      QString mName;
      QList<QString> mFields;
    };

    struct TabData
    {
      //! Constructor for TabData
      TabData() = default;
      TabData( const QString &name, const QList<QString> &fields, const QList<QgsEditFormConfig::GroupData> &groups )
        : mName( name )
        , mFields( fields )
        , mGroups( groups )
      {}
      QString mName;
      QList<QString> mFields;
      QList<QgsEditFormConfig::GroupData> mGroups;
    };

    /**
     * Types of feature form suppression after feature creation
     */
    enum FeatureFormSuppress
    {
      SuppressDefault = 0, //!< Use the application-wide setting
      SuppressOn = 1,      //!< Suppress feature form
      SuppressOff = 2      //!< Do not suppress feature form
    };
    Q_ENUM( FeatureFormSuppress )

    /**
     * The Python init code source options.
     */
    enum PythonInitCodeSource
    {
      CodeSourceNone = 0,             //!< Do not use Python code at all
      CodeSourceFile = 1,             //!< Load the Python code from an external file
      CodeSourceDialog = 2,           //!< Use the Python code provided in the dialog
      CodeSourceEnvironment = 3       //!< Use the Python code available in the Python environment
    };
    Q_ENUM( PythonInitCodeSource )

    /**
     * Data defined properties.
     * Form data defined overrides are stored in a property collection
     * and they can be retrieved using the indexes specified in this
     * enum.
     * \since QGIS 3.14
     */
    enum DataDefinedProperty
    {
      NoProperty = 0, //!< No property
      AllProperties = 1, //!< All properties for item
      Alias = 2, //!< Alias
      Editable = 3, //!< Editable state \since QGIS 3.30
    };

    /**
     * Copy constructor
     *
     * \since QGIS 3.0
     */
    QgsEditFormConfig( const QgsEditFormConfig &o );
    ~QgsEditFormConfig();

    QgsEditFormConfig &operator=( const QgsEditFormConfig &o );
    // TODO c++20 - replace with = default
    bool operator==( const QgsEditFormConfig &o ) const;

    /**
     * Adds a new element to the invisible root container in the layout.
     *
     * This is only useful in combination with EditorLayout::TabLayout.
     */
    void addTab( QgsAttributeEditorElement *data SIP_TRANSFER );

    /**
     * Returns a list of tabs for EditorLayout::TabLayout obtained from the invisible root container.
     */
    QList< QgsAttributeEditorElement * > tabs() const;

    /**
     * Clears all the tabs for the attribute editor form with EditorLayout::TabLayout.
     */
    void clearTabs();

    /**
     * Gets the invisible root container for the drag and drop designer form (EditorLayout::TabLayout).
     *
     * \since QGIS 3
     */
    QgsAttributeEditorContainer *invisibleRootContainer();

    //! Gets the active layout style for the attribute editor for this layer
    EditorLayout layout() const;

    //! Sets the active layout style for the attribute editor for this layer
    void setLayout( EditorLayout editorLayout );

    /**
     * Returns the path or URL to the .ui form. Only meaningful with EditorLayout::UiFileLayout
     */
    QString uiForm() const;

    /**
     * Set path to the .ui form.
     * When a string is provided in \a ui, the layout style will be set to EditorLayout::UiFileLayout,
     * if an empty or a null string is provided, the layout style will be set to
     * EditorLayout::GeneratedLayout.
     * If \a ui is a URL, a local copy of the file will be made and will be used to create the forms
     * \a context is provided to save error messages
     */
    void setUiForm( const QString &ui );

    /**
     * Set the editor widget config for a widget which is not for a simple field.
     *
     * ### Example
     *
     * \code{.py}
     *   editFormConfig = layer.editFormConfig()
     *   editFormConfig.setWidgetConfig( 'relation_id', { 'nm-rel': 'other_relation' } )
     *   layer.setEditFormConfig(editFormConfig)
     * \endcode
     *
     * \param widgetName  The name of the widget to configure
     * \param config      The config to set for this widget
     * \returns FALSE if a field exists with the provided widgetName. In this case
     *          QgsVectorLayer::setEditorWidgetSetup should be used.
     *
     * \see QgsVectorLayer::setEditorWidgetSetup() for field configurations.
     */
    bool setWidgetConfig( const QString &widgetName, const QVariantMap &config );

    /**
     * Gets the configuration for the editor widget with the given name.
     *
     * \param widgetName The name of the widget.
     *
     * \returns The configuration for the editor widget or an empty config if the field does not exist
     */
    QVariantMap widgetConfig( const QString &widgetName ) const;

    /**
     * Remove the configuration for the editor widget with the given name
     *
     * \param widgetName The name of the widget.
     *
     * \returns TRUE if a configuration has been removed
     */
    bool removeWidgetConfig( const QString &widgetName );

    /**
     * This returns TRUE if the field is manually set to read only or if the field
     * does not support editing like joins or virtual fields.
     */
    bool readOnly( int idx ) const;

    /**
     * If set to FALSE, the widget at the given index will be read-only.
     */
    void setReadOnly( int idx, bool readOnly = true );

    /**
     * If this returns TRUE, the widget at the given index will receive its label on the previous line
     * while if it returns FALSE, the widget will receive its label on the left hand side.
     * Labeling on top leaves more horizontal space for the widget itself.
     */
    bool labelOnTop( int idx ) const;

    /**
     * If this is set to TRUE, the widget at the given index will receive its label on
     * the previous line while if it is set to FALSE, the widget will receive its label
     * on the left hand side.
     * Labeling on top leaves more horizontal space for the widget itself.
     */
    void setLabelOnTop( int idx, bool onTop );

    /**
     * If this returns TRUE, the widget at the given \a index will remember the previously
     * entered value from this QGIS session when creating new features.
     *
     * \see setReuseLastValue()
     * \since QGIS 3.20
     */
    bool reuseLastValue( int index ) const;

    /**
     * Sets whether the widget at the given \a index will remember the previously
     * entered value from this QGIS session when creating new features.
     *
     * \see reuseLastValue()
     * \since QGIS 3.20
     */
    void setReuseLastValue( int index, bool reuse );

    // Python form init function stuff

    /**
     * Gets Python function for edit form initialization.
     * Will be looked up in a Python file relative to the project folder if it
     * includes a module name or if it's a pure function name it will searched
     * in the Python code set with setInitCode().
     */
    QString initFunction() const;

    /**
     * Set Python function for edit form initialization.
     * Will be looked up in a Python file relative to the project folder if it
     * includes a module name or if it's a pure function name it will searched
     * in the Python code set with setInitCode().
     */
    void setInitFunction( const QString &function );

    /**
     * Gets Python code for edit form initialization.
     */
    QString initCode() const;

    /**
     * Set Python code for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * setInitFunction().
     */
    void setInitCode( const QString &code );

    /**
     * Gets Python external file path for edit form initialization.
     */
    QString initFilePath() const;

    /**
     * Set Python external file path for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * setInitFunction().
     */
    void setInitFilePath( const QString &filePath );

    /**
     * Returns Python code source for edit form initialization
     *  (if it shall be loaded from a file, read from the
     *  provided dialog editor or inherited from the environment)
     */
    PythonInitCodeSource initCodeSource() const;

    //! Sets if Python code shall be used for edit form initialization and its origin
    void setInitCodeSource( PythonInitCodeSource initCodeSource );

    //! Type of feature form pop-up suppression after feature creation (overrides app setting)
    FeatureFormSuppress suppress() const;
    //! Sets type of feature form pop-up suppression after feature creation (overrides app setting)
    void setSuppress( FeatureFormSuppress s );

    // Serialization

    /**
     * Read XML information
     * Deserialize on project load
     */
    void readXml( const QDomNode &node, QgsReadWriteContext &context );

    /**
     * Write XML information
     * Serialize on project save
     */
    void writeXml( QDomNode &node, const QgsReadWriteContext &context ) const;

    /**
     * Deserialize drag and drop designer elements.
     * \deprecated since QGIS 3.18 use QgsAttributeEditorElement::create instead
     */
    Q_DECL_DEPRECATED QgsAttributeEditorElement *attributeEditorElementFromDomElement( QDomElement &elem, QgsAttributeEditorElement *parent, const QString &layerId = QString(), const QgsReadWriteContext &context = QgsReadWriteContext() ) SIP_DEPRECATED;

    /**
     * Create a new edit form config. Normally invoked by QgsVectorLayer
     */
    explicit QgsEditFormConfig();

    /**
     * Set data defined properties for \a fieldName to \a properties
     * \since QGIS 3.14
     */
    void setDataDefinedFieldProperties( const QString &fieldName, const QgsPropertyCollection &properties );

    /**
     * Returns data defined properties for \a fieldName
     * \since QGIS 3.14
     */
    QgsPropertyCollection dataDefinedFieldProperties( const QString &fieldName ) const;


    /**
     * Returns data defined property definitions.
     * \since QGIS 3.14
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

  private:

    /**
     * Used internally to set the fields when they change.
     * This should only be called from QgsVectorLayer for synchronization reasons
     */
    void setFields( const QgsFields &fields );

    /**
     * Will be called by friend class QgsVectorLayer
     */
    void onRelationsLoaded();

    /**
     * Used for the backwards compatibility of the api, on setting nmrel or force-suppress-popup for relations.
     * Returns TRUE when a relation instance (the first one found) has been updated.
     */
    bool legacyUpdateRelationWidgetInTabs( QgsAttributeEditorContainer *container,  const QString &widgetName, const QVariantMap &config );

  private:
    void fixLegacyConfig( QDomElement &el );

    QExplicitlySharedDataPointer<QgsEditFormConfigPrivate> d;

    friend class QgsVectorLayer;
};

#endif // QGSEDITFORMCONFIG_H
