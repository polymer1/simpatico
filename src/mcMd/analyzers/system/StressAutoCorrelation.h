#ifndef MCMD_STRESS_TENSOR_AUTO_CORRELATION_H
#define MCMD_STRESS_TENSOR_AUTO_CORRELATION_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include <mcMd/analyzers/SystemAnalyzer.h>
#include <util/space/Tensor.h>
#include <util/ensembles/EnergyEnsemble.h>
#include <util/accumulators/AutoCorrArray.h>     // member template

namespace McMd
{

   using namespace Util;

   /**
   * Periodically write (tensor) StressTensor to file.
   *
   * Typename SystemType can be McSystem or MdSystem.
   *
   * \ingroup DdMd_Analyzer_Module
   */
   template <class SystemType>
   class StressAutoCorrelation : public SystemAnalyzer<SystemType>
   {
   
   public:
   
      /**
      * Constructor.
      *
      * \param system parent SystemType object. 
      */
      StressAutoCorrelation(SystemType& system);
   
      /**
      * Destructor.
      */
      virtual ~StressAutoCorrelation()
      {} 
   
      /**
      * Read dumpPrefix and interval.
      *
      * \param in input parameter file
      */
      virtual void readParameters(std::istream& in);
   
      /**
      * Load internal state from an archive.
      *
      * \param ar input/loading archive
      */
      virtual void loadParameters(Serializable::IArchive &ar);

      /**
      * Save internal state to an archive.
      *
      * \param ar output/saving archive
      */
      virtual void save(Serializable::OArchive &ar);

      /**
      * Serialize to/from an archive. 
      *
      * \param ar      saving or loading archive
      * \param version archive version id
      */
      template <class Archive>
      void serialize(Archive& ar, const unsigned int version);
  
      /**
      * Setup accumulator.
      */
      virtual void setup();
  
      /**
      * Sample virial stress to accumulators
      *
      * \param iStep MD or MC step index
      */
      virtual void sample(long iStep);

      /**
      * Dump configuration to file
      */
      virtual void output();

   private:
 
      /// Output file stream
      std::ofstream  outputFile_;
      
      /// Statistical accumulator.
      AutoCorrArray<double, double>  accumulator_;

      /// Thermodynamic temperature
      double  temperature_;

      /// Number of samples per block average output
      double  temperature_;

      /// Number of samples per block average output
      int  capacity_;

      /// Has readParam been called?
      long  isInitialized_;

      using SystemAnalyzer<SystemType>::readInterval;
      using SystemAnalyzer<SystemType>::readOutputFileName;
      using SystemAnalyzer<SystemType>::read;
      using SystemAnalyzer<SystemType>::writeParam;
      using SystemAnalyzer<SystemType>::loadParameter;
      using SystemAnalyzer<SystemType>::isAtInterval;
      using SystemAnalyzer<SystemType>::outputFileName;
      using SystemAnalyzer<SystemType>::fileMaster;
      using SystemAnalyzer<SystemType>::system;
   
   };

   /*
   * Constructor.
   */
   template <class SystemType>
   StressAutoCorrelation<SystemType>::StressAutoCorrelation(SystemType& system)
    : SystemAnalyzer<SystemType>(system),
      outputFile_(),
      accumulator_(),
      temperature_(1),
      capacity_(-1),
      isInitialized_(false)
   {}

   /*
   * Read parameters and initialize.
   */
   template <class SystemType>
   void StressAutoCorrelation<SystemType>::readParameters(std::istream& in)
   {
      readInterval(in);
      readOutputFileName(in);
      read(in,"temperature", temperature_);
      read(in,"capacity", capacity_);

      accumulator_.setParam(9, capacity_);
      accumulator_.clear();

      isInitialized_ = true;
   }

   /*
   * Load state from an archive.
   */
   template <class SystemType>
   void StressAutoCorrelation<SystemType>::loadParameters(Serializable::IArchive& ar)
   {
      Analyzer::loadParameters(ar);

      loadParameter(ar, "temperature", temperature_);
      loadParameter(ar, "capacity", capacity_);
      ar & accumulator_;

      if (accumulator_.bufferCapacity() != capacity_) {
         UTIL_THROW("Inconsistent values of capacity");
      }

      isInitialized_ = true;
   }

   /*
   * Save state to archive.
   */
   template <class SystemType>
   void StressAutoCorrelation<SystemType>::save(Serializable::OArchive& ar)
   { ar & *this; }


   /*
   * Serialize to/from an archive. 
   */
   template <class SystemType>
   template <class Archive>
   void StressAutoCorrelation<SystemType>::serialize(Archive& ar, const unsigned int version)
   {
      Analyzer::serialize(ar, version);
      ar & temperature_;
      ar & capacity_;
      ar & accumulator_;
   }

   /*
   * Set up immediately before simulation.
   */
   template <class SystemType>
   void StressAutoCorrelation<SystemType>::setup() 
   {
      if (!isInitialized_) {
         UTIL_THROW("Object not initialized");
      }  
      accumulator_.clear(); 
   }

   /* 
   * Evaluate pressure, and add to accumulator.
   */
   template <class SystemType>
   void StressAutoCorrelation<SystemType>::sample(long iStep)
   {
      if (isAtInterval(iStep)){

         double pressure;
         double volume;

         SystemType& sys=system(); 
         volume = sys.boundary().volume();

         DArray<double> elements;
         elements.allocate(9);

         Tensor total;
         Tensor virial;
         Tensor kinetic;

         sys.computeVirialStress(virial);
         sys.computeKineticStress(kinetic);
         total.add(virial, kinetic);

         pressure = (total(0,0)+total(1,1)+total(2,2)) / 3.0;

         elements[0] = (total(0,0) - pressure) * sqrt(volume/(10.0 * temperature_));
         elements[1] = (total(0,1) + total(1,0)) / 2.0 * sqrt(volume/(10.0 * temperature_));
         elements[2] = (total(0,2) + total(2,0)) / 2.0 * sqrt(volume/(10.0 * temperature_));
         elements[3] = elements[1];
         elements[4] = (total(1,1) - pressure) * sqrt(volume/(10.0 * temperature_));
         elements[5] = (total(1,2) + total(2,1)) / 2.0 * sqrt(volume/(10.0 * temperature_));
         elements[6] = elements[2];
         elements[7] = elements[5];
         elements[8] = (total(2,2) - pressure) * sqrt(volume/(10.0 * temperature_));

         accumulator_.sample(elements);
     }
   }

   /*
   * Output results to file after simulation is completed.
   */
   template <class SystemType>
   void StressAutoCorrelation<SystemType>::output() 
   {
      // Write parameters to *.prm file
      fileMaster().openOutputFile(outputFileName(".prm"), outputFile_);
      writeParam(outputFile_); 
      outputFile_ << std::endl;
      outputFile_ << "bufferCapacity  " << accumulator_.bufferCapacity() << std::endl;
      outputFile_ << "nSample         " << accumulator_.nSample() << std::endl;
      outputFile_ << std::endl;
      outputFile_ << "average   " << accumulator_.average() << std::endl;
      outputFile_ << std::endl;
      outputFile_.close();

      // Write autocorrelation function to data file
      system().simulation().fileMaster().openOutputFile(outputFileName(".corr"), outputFile_);
      accumulator_.output(outputFile_);
      outputFile_.close(); 
   }

}
#endif 
