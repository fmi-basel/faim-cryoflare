include(../common.pri)

DEFINES += INSPECT_BASEDESIGN

INCLUDEPATH += \
    . \
    items \
    bands \
    base \
    objectinspector \
    databrowser

SOURCES += \
    bands/lrpageheader.cpp \
    bands/lrpagefooter.cpp \
    bands/lrreportheader.cpp \
    bands/lrreportfooter.cpp \
    bands/lrdataband.cpp \
    bands/lrgroupbands.cpp \
    bands/lrsubdetailband.cpp \
    bands/lrtearoffband.cpp \
    objectinspector/propertyItems/lrstringpropitem.cpp \
    objectinspector/propertyItems/lrrectproptem.cpp \
    objectinspector/propertyItems/lrintpropitem.cpp \
    objectinspector/propertyItems/lrenumpropitem.cpp \
    objectinspector/propertyItems/lrboolpropitem.cpp \
    objectinspector/propertyItems/lrflagspropitem.cpp \
    objectinspector/propertyItems/lrfontpropitem.cpp \
    objectinspector/propertyItems/lrimagepropitem.cpp \
    objectinspector/propertyItems/lrqrealpropitem.cpp \
    objectinspector/propertyItems/lrcolorpropitem.cpp \
    objectinspector/propertyItems/lrdatasourcepropitem.cpp \
    objectinspector/propertyItems/lrgroupfieldpropitem.cpp \
    objectinspector/propertyItems/lrcontentpropitem.cpp \
    objectinspector/editors/lrtextitempropertyeditor.cpp \
    objectinspector/editors/lrcomboboxeditor.cpp \
    objectinspector/editors/lrcheckboxeditor.cpp \
    objectinspector/editors/lrbuttonlineeditor.cpp \
    objectinspector/editors/lrfonteditor.cpp \
    objectinspector/editors/lrimageeditor.cpp \
    objectinspector/editors/lrcoloreditor.cpp \
    objectinspector/lrbasedesignobjectmodel.cpp \
    objectinspector/lrobjectinspectorwidget.cpp \
    objectinspector/lrobjectitemmodel.cpp \
    objectinspector/lrobjectpropitem.cpp \
    objectinspector/lrpropertydelegate.cpp \
    objectsbrowser/lrobjectbrowser.cpp \
    databrowser/lrdatabrowser.cpp \
    databrowser/lrsqleditdialog.cpp \
    databrowser/lrconnectiondialog.cpp \
    databrowser/lrvariabledialog.cpp \
    databrowser/lrdatabrowsertree.cpp \
    serializators/lrxmlqrectserializator.cpp \
    serializators/lrxmlbasetypesserializators.cpp \
    serializators/lrxmlreader.cpp \
    serializators/lrxmlwriter.cpp \
    items/lrsubitemparentpropitem.cpp \
    items/lralignpropitem.cpp \
    items/lrhorizontallayout.cpp \
    items/editors/lritemeditorwidget.cpp \
    items/editors/lrfonteditorwidget.cpp \
    items/editors/lrtextalignmenteditorwidget.cpp \
    items/editors/lritemsaligneditorwidget.cpp \
    items/editors/lritemsborderseditorwidget.cpp \
    items/lrsimpletagparser.cpp \
    items/lrimageitem.cpp \
    items/lrtextitemeditor.cpp \
    items/lrshapeitem.cpp \
    items/lrtextitem.cpp \
    lrbanddesignintf.cpp \
    lrpageitemdesignintf.cpp \
    lrpagedesignintf.cpp \
    lrbandsmanager.cpp \
    lrglobal.cpp \
    lritemdesignintf.cpp \
    lrdatadesignintf.cpp \
    lrreportdesignwidget.cpp \
    lrbasedesignintf.cpp \
    lrreportengine.cpp \
    lrdatasourcemanager.cpp \
    lrreportdesignwindow.cpp \
    lrreportrender.cpp \
    lrscriptenginemanager.cpp \
    lrpreviewreportwindow.cpp \
    lrpreviewreportwidget.cpp \
    lrgraphicsviewzoom.cpp \
    lrvariablesholder.cpp \
    lrgroupfunctions.cpp \
    lrsimplecrypt.cpp \
    lraboutdialog.cpp \
    lrsettingdialog.cpp \
    scriptbrowser/lrscriptbrowser.cpp \
    lritemscontainerdesignitf.cpp

contains(CONFIG, staticlib){
    SOURCES += lrfactoryinitializer.cpp
}
    
contains(CONFIG, zint){
    SOURCES += items/lrbarcodeitem.cpp
}

HEADERS += \
    base/lrsingleton.h \
    base/lrsimpleabstractfactory.h \
    base/lrattribsabstractfactory.h \
    bands/lrpageheader.h \
    bands/lrpagefooter.h \
    bands/lrreportheader.h \
    bands/lrreportfooter.h \
    bands/lrdataband.h \
    bands/lrtearoffband.h \
    bands/lrsubdetailband.h \
    bands/lrgroupbands.h \
    databrowser/lrdatabrowser.h \
    databrowser/lrsqleditdialog.h \
    databrowser/lrconnectiondialog.h \
    databrowser/lrvariabledialog.h \
    databrowser/lrdatabrowsertree.h \
    serializators/lrserializatorintf.h \
    serializators/lrstorageintf.h \
    serializators/lrxmlqrectserializator.h \
    serializators/lrxmlserializatorsfactory.h \
    serializators/lrxmlbasetypesserializators.h \
    serializators/lrxmlreader.h \
    serializators/lrxmlwriter.h \
    objectinspector/propertyItems/lrstringpropitem.h \
    objectinspector/propertyItems/lrrectproptem.h \
    objectinspector/propertyItems/lrdatasourcepropitem.h \
    objectinspector/propertyItems/lrfontpropitem.h \
    objectinspector/propertyItems/lrimagepropitem.h \
    objectinspector/propertyItems/lrintpropitem.h \
    objectinspector/propertyItems/lrenumpropitem.h \
    objectinspector/propertyItems/lrboolpropitem.h \
    objectinspector/propertyItems/lrflagspropitem.h \
    objectinspector/propertyItems/lrgroupfieldpropitem.h \
    objectinspector/propertyItems/lrcontentpropitem.h \
    objectinspector/propertyItems/lrqrealpropitem.h \
    objectinspector/propertyItems/lrcolorpropitem.h \
    objectinspector/editors/lrtextitempropertyeditor.h \
    objectinspector/editors/lrcomboboxeditor.h \
    objectinspector/editors/lrcheckboxeditor.h \
    objectinspector/editors/lrbuttonlineeditor.h \
    objectinspector/editors/lrimageeditor.h \
    objectinspector/editors/lrcoloreditor.h \
    objectinspector/editors/lrfonteditor.h \
    objectinspector/lrbasedesignobjectmodel.h \
    objectinspector/lrobjectinspectorwidget.h \
    objectinspector/lrobjectitemmodel.h \
    objectinspector/lrobjectpropitem.h \
    objectinspector/lrpropertydelegate.h \
    objectsbrowser/lrobjectbrowser.h \
    items/editors/lritemeditorwidget.h \
    items/editors/lrfonteditorwidget.h \
    items/editors/lrtextalignmenteditorwidget.h \
    items/editors/lritemsaligneditorwidget.h \
    items/editors/lritemsborderseditorwidget.h \
    items/lrtextitem.h \
    items/lrsubitemparentpropitem.h \
    items/lralignpropitem.h \
    items/lrhorizontallayout.h \
    items/lrtextitemeditor.h \
    items/lrshapeitem.h \
    items/lrimageitem.h \
    items/lrsimpletagparser.h \
    lrfactoryinitializer.h \
    lrbanddesignintf.h \
    lrpageitemdesignintf.h \
    lrbandsmanager.h \
    lrglobal.h \
    lrdatadesignintf.h \
    lrcollection.h \
    lrpagedesignintf.h \
    lrreportdesignwidget.h \
    lrreportengine_p.h \
    lrdatasourcemanager.h \
    lrreportdesignwindow.h \
    lrreportrender.h \
    lrpreviewreportwindow.h \
    lrpreviewreportwidget.h \
    lrpreviewreportwidget_p.h \
    lrgraphicsviewzoom.h \
    lrbasedesignintf.h \
    lritemdesignintf.h \
    lrdesignelementsfactory.h \
    lrscriptenginemanager.h \
    lrvariablesholder.h \
    lrgroupfunctions.h \
    lrreportengine.h \
    lrdatasourcemanagerintf.h \
    lrscriptenginemanagerintf.h \
    lrsimplecrypt.h \
    lraboutdialog.h \
    lrcallbackdatasourceintf.h \
    lrsettingdialog.h \
    lrpreviewreportwidget_p.h \
    scriptbrowser/lrscriptbrowser.h \
    lritemscontainerdesignitf.h

contains(CONFIG, staticlib){
    HEADERS += lrfactoryinitializer.h
}

contains(CONFIG,zint){
    HEADERS += items/lrbarcodeitem.h
}

FORMS += \
    databrowser/lrsqleditdialog.ui \
    databrowser/lrconnectiondialog.ui \
    databrowser/lrdatabrowser.ui \
    databrowser/lrvariabledialog.ui \
    objectinspector/editors/ltextitempropertyeditor.ui \
    lrpreviewreportwindow.ui \
    lrpreviewreportwidget.ui \
    items/lrtextitemeditor.ui \
    lraboutdialog.ui \
    lrsettingdialog.ui \
    scriptbrowser/lrscriptbrowser.ui \

RESOURCES += \
    objectinspector/lobjectinspector.qrc \
    databrowser/lrdatabrowser.qrc \
    report.qrc \
    items/items.qrc \
    scriptbrowser/lrscriptbrowser.qrc

