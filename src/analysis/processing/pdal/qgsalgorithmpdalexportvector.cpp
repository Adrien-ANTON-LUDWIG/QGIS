/***************************************************************************
                         qgsalgorithmpdalexportvector.cpp
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdalexportvector.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalExportVectorAlgorithm::name() const
{
  return QStringLiteral( "exportvector" );
}

QString QgsPdalExportVectorAlgorithm::displayName() const
{
  return QObject::tr( "Export to vector" );
}

QString QgsPdalExportVectorAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalExportVectorAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudconversion" );
}

QStringList QgsPdalExportVectorAlgorithm::tags() const
{
  return QObject::tr( "export,vector,attribute,create" ).split( ',' );
}

QString QgsPdalExportVectorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports point cloud data to a vector layer with 3D points (a GeoPackage), optionally with extra attributes." );
}

QgsPdalExportVectorAlgorithm *QgsPdalExportVectorAlgorithm::createInstance() const
{
  return new QgsPdalExportVectorAlgorithm();
}

void QgsPdalExportVectorAlgorithm::initAlgorithm( const QVariantMap & )
{
  // for now we use hardcoded list of attributes, as currently there is
  // no way to retrieve them from the point cloud layer without indexing
  // it first. Later we will add a corresponding parameter type for it.
  QStringList attributes =
  {
    QStringLiteral( "X" ),
    QStringLiteral( "Y" ),
    QStringLiteral( "Z" ),
    QStringLiteral( "Intensity" ),
    QStringLiteral( "ReturnNumber" ),
    QStringLiteral( "NumberOfReturns" ),
    QStringLiteral( "Classification" ),
    QStringLiteral( "GpsTime" )
  };

  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "ATTRIBUTE" ), QObject::tr( "Attribute" ), attributes, false, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterVectorDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output vector" ), QgsProcessing::TypeVectorPoint ) );
}

QStringList QgsPdalExportVectorAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = { QStringLiteral( "to_vector" ),
                       QStringLiteral( "--input=%1" ).arg( layer->source() ),
                       QStringLiteral( "--output=%1" ).arg( outputFile ),
                     };

  if ( parameters.value( QStringLiteral( "ATTRIBUTE" ) ).isValid() )
  {
    const QString attribute = parameterAsEnumString( parameters, QStringLiteral( "ATTRIBUTE" ), context );
    args << QStringLiteral( "--attribute=%1" ).arg( attribute );
  }

  addThreadsParameter( args );
  return args;
}

///@endcond
