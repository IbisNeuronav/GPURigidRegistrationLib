#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

#include "utils/vtkXFMReader.h"
#include "utils/vtkXFMWriter.h"

#include <string> 
#include <iostream>
#include <cmath>

// accuracy threshold to pass the test in mm
#define THRESHOLD 3 

int runAccuracyTest(const char * dataDir, const char* outputTransformName)
{
    vtkTransform * outputTransform = vtkTransform::New();
    vtkMatrix4x4 * outputMat = vtkMatrix4x4::New();
    vtkXFMReader * reader = vtkXFMReader::New();

    if( reader->CanReadFile(outputTransformName) )
    {
        reader->SetFileName(outputTransformName);
        reader->SetMatrix(outputMat);
        reader->Update();
    }
        
    outputTransform->SetMatrix(outputMat);

    const unsigned int numberOfPoints = 14;
    double points[numberOfPoints][3] = {{-0.5188,   -47.0008,   43.7264},
                            {-3.32498,  -46.8987,   7.3057 },
                            {8.64599,   -47.0064,   -13.1793 },
                            {-0.789869, -58.0043,   46.9116 },
                            {-2.71422,  -57.9044,   5.92169 },
                            {28.9042,   -36.332,    29.4946 },
                            {9.23037,   -36.0704,   17.7571 },
                            {-0.989595, -36.0032,   48.5715 },
                            {22.8003,   -28.2416,   21.2269 },
                            {-0.451716, -27.9804,   33.1016 },
                            {-0.965692, -55.0044,   48.2591 },
                            {-2.46232,  -54.9172,   11.252 },
                            {27.2235,   -42.621,    2.07393 },
                            {-31.7794,  -49.4114,   4.44408 }};

    double averageAccuracy = 0;

    const unsigned int numberOfTransforms = 10;
    for( unsigned int i = 1; i < numberOfTransforms + 1; i++ )
    {
        std::string transformName = std::string(dataDir) + "/output-" + std::to_string(i) + ".xfm";

        vtkTransform * transform = vtkTransform::New();
        vtkMatrix4x4 * mat = vtkMatrix4x4::New();
        if( reader->CanReadFile(transformName.c_str()) )
        {
            reader->SetFileName(transformName.c_str());
            reader->SetMatrix(mat);
            reader->Update();
        }

        transform->SetMatrix(mat);

        double meanDistance = 0;

        for( size_t j = 0; j < numberOfPoints; j++ )
        {
            double * goldStanradPoint = transform->TransformDoublePoint(points[j]);
            double * outputPoint = outputTransform->TransformDoublePoint(points[j]);
            double dist = 0;

            for( size_t k = 0; k < 3; k++ )
            {
                dist += (outputPoint[k] - goldStanradPoint[k]) * (outputPoint[k] - goldStanradPoint[k]);
            }

            dist = std::sqrt(dist);
            meanDistance += dist;
        }

        meanDistance = meanDistance / numberOfPoints;
        averageAccuracy += meanDistance;
    }

    averageAccuracy = averageAccuracy / numberOfTransforms;

    if( averageAccuracy < THRESHOLD )
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}

