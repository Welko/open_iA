/*************************************  open_iA  ************************************ *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2017  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
*                          J. Weissenböck, Artem & Alexander Amirkhanov, B. Fröhler   *
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
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#include "pch.h"
#include "iAGeometricTransformations.h"

#include "defines.h"          // for DIM
#include "iAConnector.h"
#include "iAProgress.h"
#include "iATypedCallHelper.h"

#include <itkBSplineInterpolateImageFunction.h>
#include <itkChangeInformationImageFilter.h>
#include <itkImageIOBase.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkWindowedSincInterpolateImageFunction.h>

#include <vtkImageData.h>

#include <QLocale>

const QString iAGeometricTransformations::InterpLinear("Linear");
const QString iAGeometricTransformations::InterpNearestNeighbour("Nearest Neighbour");
const QString iAGeometricTransformations::InterpBSpline("BSpline");
const QString iAGeometricTransformations::InterpWindowedSinc("Windowed Sinc");


/**
* template extractImage
* 
* This template extracts a subimage, allowing a reduction of dimensions. 
* \param	indexX	The index x coordinate. 
* \param	indexY	The index y coordinate. 
* \param	indexZ	The index z coordinate. 
* \param	sizeX	The size x coordinate. 
* \param	sizeY	The size y coordinate. 
* \param	sizeZ	The size z coordinate. 
* \param	p		If non-null, the. 
* \param	image	If non-null, the image.
* \param	T		Template datatype.
* \return	int Status-Code. 
*/
template<class T> 
int extractImage_template( double indexX, double indexY, double indexZ, double sizeX, double sizeY, double sizeZ, iAProgress* p, iAConnector* image  )
{
	typedef itk::Image< T, DIM > InputImageType;
	typedef itk::Image< T, DIM > OutputImageType;
	typedef itk::ExtractImageFilter< InputImageType, OutputImageType > EIFType;
	typename EIFType::Pointer filter = EIFType::New();

	typename EIFType::InputImageRegionType::SizeType size; size[0] = sizeX; size[1] = sizeY; size[2] = sizeZ;
	typename EIFType::InputImageRegionType::IndexType index; index[0] = indexX; index[1] = indexY; index[2] = indexZ;
	typename EIFType::InputImageRegionType region; region.SetIndex(index); region.SetSize(size);

	filter->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );

	filter->SetExtractionRegion(region);
	p->Observe( filter );
	filter->Update();

	//change the output image information - offset change to zero
	typename OutputImageType::IndexType idx; idx.Fill(0);
	typename OutputImageType::PointType origin; origin.Fill(0);
	typename OutputImageType::SizeType outsize; outsize[0] = sizeX;	outsize[1] = sizeY;	outsize[2] = sizeZ;
	typename OutputImageType::RegionType outreg;
	outreg.SetIndex(idx); 
	outreg.SetSize(outsize);
	typename OutputImageType::Pointer refimage = OutputImageType::New();
	refimage->SetRegions(outreg);
	refimage->SetOrigin(origin);
	refimage->SetSpacing(filter->GetOutput()->GetSpacing());
	refimage->Allocate();

	typedef itk::ChangeInformationImageFilter<OutputImageType> CIIFType;
	typename CIIFType::Pointer changefilter = CIIFType::New();
	changefilter->SetInput(filter->GetOutput());
	changefilter->UseReferenceImageOn();
	changefilter->SetReferenceImage(refimage);
	changefilter->SetChangeRegion(1);
	changefilter->Update( );

	image->SetImage( changefilter->GetOutput() );
	image->Modified();

	filter->ReleaseDataFlagOn();

	return EXIT_SUCCESS;
}

/**
* template resampler
* 
* This template resamples a grid. 
* \param	originX		The origin x coordinate. 
* \param	originY		The origin y coordinate. 
* \param	originZ		The origin z coordinate. 
* \param	spacingX	The spacing x coordinate. 
* \param	spacingY	The spacing y coordinate. 
* \param	spacingZ	The spacing z coordinate. 
* \param	sizeX		The size x coordinate. 
* \param	sizeY		The size y coordinate. 
* \param	sizeZ		The size z coordinate. 
* \param	p			If non-null, the. 
* \param	image		If non-null, the image. 
* \param				The. 
* \return	int Status-Code. 
*/
template<class T> 
int resampler_template(
	double originX, double originY, double originZ,
	double spacingX, double spacingY, double spacingZ,
	double sizeX, double sizeY, double sizeZ,
	QString const & interpolator,
	iAProgress* p, iAConnector* image  )
{
	typedef itk::Image< T, DIM > InputImageType;
	typedef itk::Image< T, DIM > OutputImageType;
	typedef itk::ResampleImageFilter< InputImageType, OutputImageType >    ResampleFilterType;
	typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();

	typename ResampleFilterType::OriginPointType origin; origin[0] = originX; origin[1] = originY; origin[2] = originZ;
	typename ResampleFilterType::SpacingType spacing; spacing[0] = spacingX; spacing[1] = spacingY; spacing[2] = spacingZ;
	typename ResampleFilterType::SizeType size; size[0] = sizeX; size[1] = sizeY; size[2] = sizeZ;

	if (interpolator == iAGeometricTransformations::InterpLinear)
	{
		typedef itk::LinearInterpolateImageFunction<InputImageType, double> InterpolatorType;
		typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolator == iAGeometricTransformations::InterpNearestNeighbour)
	{
		typedef itk::NearestNeighborInterpolateImageFunction<InputImageType, double> InterpolatorType;
		typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolator == iAGeometricTransformations::InterpBSpline)
	{
		typedef itk::BSplineInterpolateImageFunction<InputImageType, double> InterpolatorType;
		typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolator == iAGeometricTransformations::InterpWindowedSinc)
	{
		typedef itk::Function::HammingWindowFunction<3> WindowFunctionType;
		typedef itk::ZeroFluxNeumannBoundaryCondition<InputImageType> ConditionType;
		typedef itk::WindowedSincInterpolateImageFunction<
			InputImageType, 3,
			WindowFunctionType,
			ConditionType,
			double> InterpolatorType;
		typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	resampler->SetInput( dynamic_cast< InputImageType * >( image->GetITKImage() ) );
	resampler->SetOutputOrigin( origin );
	resampler->SetOutputSpacing( spacing );
	resampler->SetSize( size );
	resampler->SetDefaultPixelValue( 0 );

	p->Observe( resampler );
	resampler->Update( );
	image->SetImage( resampler->GetOutput() );
	image->Modified();

	resampler->ReleaseDataFlagOn();

	return EXIT_SUCCESS;
}

iAGeometricTransformations::iAGeometricTransformations( QString fn, iAGeometricTransformationType fid, vtkImageData* i, vtkPolyData* p, iALogger* logger, QObject* parent  )
	: iAAlgorithm( fn, i, p, logger, parent ), m_operation(fid)
{
}


void iAGeometricTransformations::performWork()
{
	iAConnector::ITKScalarPixelType itkType = getConnector()->GetITKScalarPixelType();
	switch (m_operation)
	{
	case EXTRACT_IMAGE:
		ITK_TYPED_CALL(extractImage_template, itkType,
			originX, originY, originZ, sizeX, sizeY, sizeZ, getItkProgress(), getConnector());
		break;
	case RESAMPLER:
		ITK_TYPED_CALL(resampler_template, itkType,
			originX, originY, originZ,
			spacingX, spacingY, spacingZ,
			sizeX, sizeY, sizeZ,
			interpolator,
			getItkProgress(), getConnector());
		break;
	default:
		addMsg(tr("  unknown filter type"));
	}
}
