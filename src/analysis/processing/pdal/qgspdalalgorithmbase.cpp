/***************************************************************************
                         qgspdalalgorithmbase.cpp
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

#include "qgspdalalgorithmbase.h"

#include "qgsapplication.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

//
// QgsPdalAlgorithmBase
//

void QgsPdalAlgorithmBase::setOutputValue( const QString &name, const QVariant &value )
{
  mOutputValues.insert( name, value );
}

QString QgsPdalAlgorithmBase::wrenchExecutableBinary() const
{
  QString wrenchExecutable = QProcessEnvironment::systemEnvironment().value( QStringLiteral( "QGIS_WRENCH_EXECUTABLE" ) );
  if ( wrenchExecutable.isEmpty() )
  {
#if defined(Q_OS_WIN)
    wrenchExecutable = QgsApplication::libexecPath() + "pdal_wrench.exe";
#else
    wrenchExecutable = QgsApplication::libexecPath() + "pdal_wrench";
#endif
  }
  return QString( wrenchExecutable );
}

void QgsPdalAlgorithmBase::addThreadsParameter( QStringList &arguments )
{
  QgsSettings settings;
  int threads = settings.value( QStringLiteral( "/Processing/Configuration/MAX_THREADS" ), 0 ).toInt();

  if ( threads )
  {
    arguments << QStringLiteral( "--threads=%1" ).arg( threads );
  }
}

QStringList QgsPdalAlgorithmBase::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );
  Q_UNUSED( feedback );
  return QStringList();
}

QVariantMap QgsPdalAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QStringList processArgs = createArgumentLists( parameters, context, feedback );
  const QString wrenchPath = wrenchExecutableBinary();

  feedback->pushCommandInfo( QObject::tr( "wrench command: " ) + wrenchPath + ' ' + processArgs.join( ' ' ) );

  double progress = 0;
  QString buffer;

  QgsBlockingProcess wrenchProcess( wrenchPath, processArgs );
  wrenchProcess.setStdErrHandler( [ = ]( const QByteArray & ba )
  {
    feedback->reportError( ba.trimmed() );
  } );
  wrenchProcess.setStdOutHandler( [ =, &progress, &buffer ]( const QByteArray & ba )
  {
    QString data( ba );

    QRegularExpression re( "\\.*(\\d+)?\\.*$" );
    QRegularExpressionMatch match = re.match( data );
    if ( match.hasMatch() )
    {
      QString value = match.captured( 1 );
      if ( !value.isEmpty() )
      {
        progress = value.toInt();
        if ( progress != 100 )
        {
          int pos = match.capturedEnd();
          QString points = data.mid( pos );
          progress += 2.5 * points.size();
        }
      }
      else
      {
        progress += 2.5 * data.size();
      }

      feedback->setProgress( progress );
    }

    buffer.append( data );
    if ( buffer.contains( '\n' ) || buffer.contains( '\r' ) )
    {
      QStringList parts = buffer.split( '\n' );
      if ( parts.size() == 0 )
      {
        parts = buffer.split( '\r' );
      }

      for ( int i = 0; i < parts.size() - 1; i++ )
        feedback->pushConsoleInfo( parts.at( i ) );
      buffer = parts.at( parts.size() - 1 );
    }
  } );

  const int res = wrenchProcess.run( feedback );
  if ( feedback->isCanceled() && res != 0 )
  {
    feedback->pushInfo( QObject::tr( "Process was canceled and did not complete" ) )  ;
  }
  else if ( !feedback->isCanceled() && wrenchProcess.exitStatus() == QProcess::CrashExit )
  {
    throw QgsProcessingException( QObject::tr( "Process was unexpectedly terminated" ) );
  }
  else if ( res == 0 )
  {
    feedback->pushInfo( QObject::tr( "Process completed successfully" ) );
  }
  else if ( wrenchProcess.processError() == QProcess::FailedToStart )
  {
    throw QgsProcessingException( QObject::tr( "Process %1 failed to start. Either %1 is missing, or you may have insufficient permissions to run the program." ).arg( wrenchPath ) );
  }
  else
  {
    throw QgsProcessingException( QObject::tr( "Process returned error code %1" ).arg( res ) );
  }

  QVariantMap outputs;
  QgsProcessingOutputDefinitions outDefinitions = outputDefinitions();
  for ( const QgsProcessingOutputDefinition *output : outDefinitions )
  {
    QString outputName = output->name();
    if ( parameters.contains( outputName ) )
    {
      outputs.insert( outputName, parameters.value( outputName ) );
    }
  }

  QMap<QString, QVariant>::const_iterator it = mOutputValues.constBegin();
  while ( it != mOutputValues.constEnd() )
  {
    outputs[ it.key() ] = it.value();
    ++it;
  }

  return outputs;
}

///@endcond
