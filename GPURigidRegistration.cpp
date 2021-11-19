#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageMaskSpatialObject.h"
#include "gpu_rigidregistration.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

#include "utils/vtkXFMReader.h"
#include "utils/vtkXFMWriter.h"

#include "vtksys/CommandLineArguments.hxx"
#include <iostream>

int main(int argc, char* argv[])
{
    // Check command line arguments.
    bool printHelp = false;
    std::string movingImageFileName;
    std::string fixedImageFileName;
    std::string initialTransformFileName;
    std::string outputTransformFileName;
    std::string fixedMaskFileName;
    std::string movingMaskFileName;
    
    vtksys::CommandLineArguments args;
    args.Initialize(argc, argv);
    args.StoreUnusedArguments(true);

    args.AddArgument("--help", vtksys::CommandLineArguments::NO_ARGUMENT, &printHelp, "Print this help.");
    args.AddArgument("--initial-transform", vtksys::CommandLineArguments::SPACE_ARGUMENT, &initialTransformFileName, "Name of the initial transform file (*.xfm).");
    args.AddArgument("--output-transform", vtksys::CommandLineArguments::SPACE_ARGUMENT, &outputTransformFileName, "Name of the output transform file (default outputTransform.xfm).");
    args.AddArgument("--fixed-mask", vtksys::CommandLineArguments::SPACE_ARGUMENT, &fixedMaskFileName, "Name of the fixed image mask file.");
    args.AddArgument("--moving-mask", vtksys::CommandLineArguments::SPACE_ARGUMENT, &movingMaskFileName, "Name of the moving image mask file.");

    args.Parse();
    if( printHelp )
    {
        std::cerr << "Problem parsing arguments." << std::endl;
        std::cout << "Help: " << args.GetHelp() << std::endl;
        return EXIT_FAILURE;
    }

    char ** newArgv = nullptr;
    int newArgc = 0;
    args.GetUnusedArguments(&newArgc, &newArgv);

    if( newArgc < 3 )
    {
        std::cout << "Usage: " << argv[0] << " movingImage fixedImage [options]" << std::endl;
        std::cout << "Options: " << args.GetHelp() << std::endl;
        return EXIT_FAILURE;
    }

    movingImageFileName = newArgv[1];
    fixedImageFileName = newArgv[2];

    if( outputTransformFileName.empty() )
    {
        std::cout << "No output transform file specified. Resulting transform will be written in outputTransform.h5" << std::endl;
        outputTransformFileName = "outputTransform.xfm";
    }
    
    const unsigned int Dimension = 3;
    using PixelType = float;
    using ImageType = itk::Image<PixelType, Dimension>;
    using MovingImageReaderType = itk::ImageFileReader<ImageType>;
    using FixedImageReaderType = itk::ImageFileReader<ImageType>;
    using ImageMaskType = itk::ImageMaskSpatialObject< 3 >;
    
    ImageMaskType::Pointer fixedMask = nullptr;
    ImageMaskType::Pointer movingMask = nullptr;
    
    // Reading moving image file
    MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
    movingReader->SetFileName(movingImageFileName.c_str());
    std::cout << "Reading Moving Image... " << movingImageFileName << std::endl;
    try
    {
        movingReader->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer movingImage = movingReader->GetOutput();
    
    //Reading fixed image file 
    FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
    fixedReader->SetFileName(fixedImageFileName.c_str());
    std::cout << "Reading Fixed Image... " << fixedImageFileName << std::endl;
    try
    {
        fixedReader->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    ImageType::Pointer fixedImage = fixedReader->GetOutput();

    vtkTransform * movingTransform = nullptr;

    if( !initialTransformFileName.empty() )
    {
        //read initialization transform in a vtkTransform
        std::cout << "Reading Initial Transform for Moving Image... " << initialTransformFileName << std::endl;

        movingTransform = vtkTransform::New();
        vtkMatrix4x4 * mat = vtkMatrix4x4::New();
        vtkXFMReader * reader = vtkXFMReader::New();
        if( reader->CanReadFile(initialTransformFileName.c_str()) )
        {
            reader->SetFileName(initialTransformFileName.c_str());
            reader->SetMatrix(mat);
            reader->Update();
            reader->Delete();
        }
        
        movingTransform->SetMatrix(mat);

    }

    if( !movingMaskFileName.empty() )
    {
        // Reading moving mask file
        using MaskReaderType = itk::ImageFileReader<ImageMaskType::ImageType>;
        MaskReaderType::Pointer movingMaskReader = MaskReaderType::New();
        movingMaskReader->SetFileName(movingMaskFileName.c_str());
        std::cout << "Reading Moving Mask... " << movingMaskFileName << std::endl;

        try
        {
            movingMaskReader->Update();
            movingMask = ImageMaskType::New();
            movingMask->SetImage(movingMaskReader->GetOutput());
            movingMask->Update();
        }
        catch( itk::ExceptionObject & error )
        {
            std::cerr << "Error: " << error << std::endl;
            return EXIT_FAILURE;
        }
    }

    if( !fixedMaskFileName.empty() )
    {
        // Reading fixed mask file
        using MaskReaderType = itk::ImageFileReader<ImageMaskType::ImageType>;
        MaskReaderType::Pointer fixedMaskReader = MaskReaderType::New();
        fixedMaskReader->SetFileName(fixedMaskFileName.c_str());
        std::cout << "Reading Fixed Mask... " << fixedMaskFileName << std::endl;

        try
        {
            fixedMaskReader->Update();
            fixedMask = ImageMaskType::New();
            fixedMask->SetImage(fixedMaskReader->GetOutput());
            fixedMask->Update();
        }
        catch( itk::ExceptionObject & error )
        {
            std::cerr << "Error: " << error << std::endl;
            return EXIT_FAILURE;
        }
    }

    GPU_RigidRegistration* rigidRegistrator = new GPU_RigidRegistration();

    // Initialize parameters
    rigidRegistrator->SetNumberOfPixels(128000);
    rigidRegistrator->SetOrientationSelectivity(32);
    rigidRegistrator->SetPopulationSize(100);
    rigidRegistrator->SetPercentile(0.8);
    rigidRegistrator->SetUseMask(true);

    // Set image inputs
    rigidRegistrator->SetItkSourceImage(movingImage);
    rigidRegistrator->SetItkTargetImage(fixedImage);

    if( movingTransform )
    {
        rigidRegistrator->SetSourceVtkTransform(movingTransform);
    }

    // Set transform inputs
    vtkTransform * transform = vtkTransform::New();
    rigidRegistrator->SetVtkTransform(transform);

    if( movingMask )
    {
        rigidRegistrator->SetSourceMask(movingMask);
    }

    if( fixedMask )
    {
        rigidRegistrator->SetTargetMask(fixedMask);
    }

    rigidRegistrator->runRegistration();

    // Write output transform
    vtkXFMWriter * transformWriter = vtkXFMWriter::New();
    transformWriter->SetFileName(outputTransformFileName.c_str());
    transformWriter->SetMatrix(transform->GetMatrix());
    transformWriter->Write();

    return EXIT_SUCCESS;
}

