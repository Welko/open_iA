/*********************************  open_iA 2016 06  ******************************** *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, J. Weissenb�ck, *
*                     Artem & Alexander Amirkhanov, B. Fr�hler                        *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH O� Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
 
#include "pch.h"
#include "iAThresholding.h"

#include "iAConnector.h"
#include "iAProgress.h"
#include "iATypedCallHelper.h"

#include <itkAdaptiveOtsuThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkRobustAutomaticThresholdImageFilter.h>
#include <itkRemovePeaksOtsuThresholdImageFilter.h>

#include <QLocale>

template<class T> int binary_threshold_template(double l, double u, double o, double i, iAProgress* p, iAConnector* image)
{
	typedef itk::Image< T, 3 >   InputImageType;
	typedef itk::Image< T, 3 >   OutputImageType;
	typedef itk::BinaryThresholdImageFilter<InputImageType, OutputImageType> BTIFType;
	typename BTIFType::Pointer filter = BTIFType::New();
	filter->SetLowerThreshold(T(l));
	filter->SetUpperThreshold(T(u));
	filter->SetOutsideValue(T(o));
	filter->SetInsideValue(T(i));
	filter->SetInput(dynamic_cast<InputImageType *>(image->GetITKImage()));
	p->Observe(filter);
	filter->Update();
	image->SetImage(filter->GetOutput());
	image->Modified();
	filter->ReleaseDataFlagOn();
	return EXIT_SUCCESS;
}


template<class T> 
int rats_threshold_template( double* rthresh_ptr, double pow, double o, double i, iAProgress* p, iAConnector* image  )
{
	typedef typename itk::Image< T, 3 >   InputImageType;
	typedef typename itk::Image< T, 3 >   OutputImageType;
	typedef typename itk::Image< float, 3 >   GradientImageType;
	typedef itk::GradientMagnitudeImageFilter< InputImageType, GradientImageType > GMFType;
	typename GMFType::Pointer gmfilter = GMFType::New();
	gmfilter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );
	p->Observe( gmfilter );
	gmfilter->Update(); 
	typedef typename itk::RobustAutomaticThresholdImageFilter < InputImageType, GradientImageType, OutputImageType > RATIFType;
	typename RATIFType::Pointer filter = RATIFType::New();
	filter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );
	filter->SetGradientImage(gmfilter->GetOutput());
	filter->SetOutsideValue( T(o) );
	filter->SetInsideValue( T(i) );
	filter->SetPow( pow );
	p->Observe( filter );
	filter->Update();
	*rthresh_ptr = (double)filter->GetThreshold();
	image->SetImage(filter->GetOutput());
	image->Modified();
	gmfilter->ReleaseDataFlagOn();
	filter->ReleaseDataFlagOn();
	return EXIT_SUCCESS;
}


template<class T> 
int adaptive_otsu_threshold( double r, unsigned int s, unsigned int l, unsigned int c, double b, double o, double i, iAProgress* p, iAConnector* image  )
{
	typedef itk::Image< T, 3 >   InputImageType;
	typedef itk::Image< T, 3 >   OutputImageType;
	typename InputImageType::SizeType radius;
	radius.Fill( T(r) );
	typedef itk::AdaptiveOtsuThresholdImageFilter< InputImageType, OutputImageType > AOTIFType;
	typename AOTIFType::Pointer filter = AOTIFType::New();
	filter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );
	filter->SetOutsideValue( T(o) );
	filter->SetInsideValue( T(i) );
	filter->SetNumberOfHistogramBins( T(b) );
	filter->SetNumberOfControlPoints( c );
	filter->SetNumberOfLevels( l );
	filter->SetNumberOfSamples( s);
	filter->SetRadius( radius );
	filter->Update();
	p->Observe( filter );
	filter->Update();
	image->SetImage(filter->GetOutput());
	image->Modified();
	filter->ReleaseDataFlagOn();
	return EXIT_SUCCESS;
}


template<class T> 
int otsu_threshold_template( double* othresh_ptr, double b, double o, double i, bool removepeaks, iAProgress* p, iAConnector* image  )
{
	typedef typename itk::Image< T, 3 >   InputImageType;
	typedef typename itk::Image< T, 3 >   OutputImageType;
	typedef typename itk::OtsuThresholdImageFilter < InputImageType, OutputImageType > OTIFType;
	typedef typename itk::RemovePeaksOtsuThresholdImageFilter < InputImageType, OutputImageType > RPOTIFType;

	if (removepeaks)
	{
		typename RPOTIFType::Pointer filter = RPOTIFType::New();
		filter->SetNumberOfHistogramBins( T (b) );
		filter->SetOutsideValue( T(o) );
		filter->SetInsideValue( T(i) );
		filter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );
		p->Observe( filter );
		filter->Update();
		*othresh_ptr = (double)filter->GetThreshold();
		image->SetImage(filter->GetOutput());
		image->Modified();
		filter->ReleaseDataFlagOn();
	}
	else
	{
		typename OTIFType::Pointer filter = OTIFType::New();
		filter->SetNumberOfHistogramBins( T (b) );
		filter->SetOutsideValue( T(o) );
		filter->SetInsideValue( T(i) );
		filter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );
		p->Observe( filter );
		filter->Update();
		*othresh_ptr = (double)filter->GetThreshold();
		image->SetImage(filter->GetOutput());
		image->Modified();
		filter->ReleaseDataFlagOn();
	}
	return EXIT_SUCCESS;
}


template<class T> 
int otsu_multiple_threshold_template( std::vector<double>* omthresh_ptr, double b, double n, iAProgress* p, iAConnector* image )
{
	typedef typename itk::Image< T, 3 >   InputImageType;
	typedef typename itk::Image< T, 3 >   OutputImageType;

	typedef typename itk::OtsuMultipleThresholdsImageFilter < InputImageType, OutputImageType > OMTIFType;

	typename OMTIFType::Pointer filter = OMTIFType::New();
	filter->SetNumberOfHistogramBins( T (b) );
	filter->SetNumberOfThresholds( T (n) );
	filter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );

	p->Observe( filter );

	filter->Update();

	*omthresh_ptr = filter->GetThresholds();
	image->SetImage(filter->GetOutput());
	image->Modified();

	filter->ReleaseDataFlagOn();

	return EXIT_SUCCESS;
}


iAThresholding::iAThresholding( QString fn, FilterID fid, vtkImageData* i, vtkPolyData* p, iALogger* logger, QObject* parent )
	: iAAlgorithms( fn, fid, i, p, logger, parent )
{
}


iAThresholding::~iAThresholding()
{
}


void iAThresholding::run()
{
	addMsg(tr("%1  %2 started.").arg(QLocale().toString(Start(), QLocale::ShortFormat))
		.arg(getFilterName()));
	getConnector()->SetImage(getVtkImageData()); getConnector()->Modified();
	try
	{
		switch (getFilterID())
		{
		case BINARY_THRESHOLD:
			binaryThresh(); break;
		case OTSU_MULTIPLE_THRESHOLD:
			otsuMultipleThresh(); break;
		case OTSU_THRESHOLD:
			otsuThresh(); break;
		case ADAPTIVE_OTSU_THRESHOLD:
			adaptiveOtsuThresh(); break;
		case RATS_THRESHOLD:
			ratsThresh(); break;
		default:
			addMsg(tr("unknown filter type"));
		}
	}
	catch (itk::ExceptionObject &excep)
	{
		addMsg(tr("%1  %2 terminated unexpectedly. Elapsed time: %3 ms").arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
			.arg(getFilterName())
			.arg(Stop()));
		addMsg(tr("  %1 in File %2, Line %3").arg(excep.GetDescription())
			.arg(excep.GetFile())
			.arg(excep.GetLine()));
		return;
	}
	addMsg(tr("%1  %2 finished. Elapsed time: %3 ms").arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
		.arg(getFilterName())
		.arg(Stop()));

	emit startUpdate();
}


void iAThresholding::binaryThresh()
{
	iAConnector::ITKScalarPixelType itkType = getConnector()->GetITKScalarPixelType();
	ITK_TYPED_CALL(binary_threshold_template, itkType,
		lower, upper, outer, inner, getItkProgress(), getConnector());
}


void iAThresholding::otsuMultipleThresh()
{
	iAConnector::ITKScalarPixelType itkType = getConnector()->GetITKScalarPixelType();
	ITK_TYPED_CALL(otsu_multiple_threshold_template, itkType,
		&omthreshs, bins, threshs, getItkProgress(), getConnector());
	addMsg(tr("%1  Otsu Multiple Thresholds = %2").arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
		.arg(QString::number(threshs)) );
	for (int i = 0; i< threshs; i++) addMsg(tr("    Threshold number %1 = %2").arg(i).arg(omthreshs[i]));
}


void iAThresholding::otsuThresh()
{
	iAConnector::ITKScalarPixelType itkType = getConnector()->GetITKScalarPixelType();
	ITK_TYPED_CALL(otsu_threshold_template, itkType,
		&othresh, bins, outer, inner, removepeaks, getItkProgress(), getConnector());
	addMsg(tr("%1  Otsu Threshold = %2").arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
		.arg(QString::number(othresh)) );
}


void iAThresholding::adaptiveOtsuThresh()
{
	iAConnector::ITKScalarPixelType itkType = getConnector()->GetITKScalarPixelType();
	ITK_TYPED_CALL(adaptive_otsu_threshold, itkType,
		radius, samples, levels, controlPoints, bins, outer, inner, getItkProgress(), getConnector());
}


void iAThresholding::ratsThresh()
{
	iAConnector::ITKScalarPixelType itkType = getConnector()->GetITKScalarPixelType();
	ITK_TYPED_CALL(rats_threshold_template, itkType,
		&rthresh, power, outer, inner, getItkProgress(), getConnector());
	addMsg(tr("%1  Rats Threshold = %2").arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
		.arg(QString::number(rthresh)) );
}