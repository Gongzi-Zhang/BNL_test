// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME caliTypeDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "caliType.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_caliHit(void *p = nullptr);
   static void *newArray_caliHit(Long_t size, void *p);
   static void delete_caliHit(void *p);
   static void deleteArray_caliHit(void *p);
   static void destruct_caliHit(void *p);
   static void streamer_caliHit(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::caliHit*)
   {
      ::caliHit *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::caliHit >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("caliHit", ::caliHit::Class_Version(), "caliType.h", 6,
                  typeid(::caliHit), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::caliHit::Dictionary, isa_proxy, 16,
                  sizeof(::caliHit) );
      instance.SetNew(&new_caliHit);
      instance.SetNewArray(&newArray_caliHit);
      instance.SetDelete(&delete_caliHit);
      instance.SetDeleteArray(&deleteArray_caliHit);
      instance.SetDestructor(&destruct_caliHit);
      instance.SetStreamerFunc(&streamer_caliHit);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::caliHit*)
   {
      return GenerateInitInstanceLocal((::caliHit*)nullptr);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::caliHit*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_caliCluster(void *p = nullptr);
   static void *newArray_caliCluster(Long_t size, void *p);
   static void delete_caliCluster(void *p);
   static void deleteArray_caliCluster(void *p);
   static void destruct_caliCluster(void *p);
   static void streamer_caliCluster(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::caliCluster*)
   {
      ::caliCluster *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::caliCluster >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("caliCluster", ::caliCluster::Class_Version(), "caliType.h", 19,
                  typeid(::caliCluster), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::caliCluster::Dictionary, isa_proxy, 16,
                  sizeof(::caliCluster) );
      instance.SetNew(&new_caliCluster);
      instance.SetNewArray(&newArray_caliCluster);
      instance.SetDelete(&delete_caliCluster);
      instance.SetDeleteArray(&deleteArray_caliCluster);
      instance.SetDestructor(&destruct_caliCluster);
      instance.SetStreamerFunc(&streamer_caliCluster);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::caliCluster*)
   {
      return GenerateInitInstanceLocal((::caliCluster*)nullptr);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::caliCluster*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr caliHit::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *caliHit::Class_Name()
{
   return "caliHit";
}

//______________________________________________________________________________
const char *caliHit::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::caliHit*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int caliHit::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::caliHit*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *caliHit::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::caliHit*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *caliHit::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::caliHit*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr caliCluster::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *caliCluster::Class_Name()
{
   return "caliCluster";
}

//______________________________________________________________________________
const char *caliCluster::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::caliCluster*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int caliCluster::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::caliCluster*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *caliCluster::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::caliCluster*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *caliCluster::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::caliCluster*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void caliHit::Streamer(TBuffer &R__b)
{
   // Stream an object of class caliHit.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      TObject::Streamer(R__b);
      R__b >> x;
      R__b >> y;
      R__b >> z;
      R__b >> e;
      R__b.CheckByteCount(R__s, R__c, caliHit::IsA());
   } else {
      R__c = R__b.WriteVersion(caliHit::IsA(), kTRUE);
      TObject::Streamer(R__b);
      R__b << x;
      R__b << y;
      R__b << z;
      R__b << e;
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_caliHit(void *p) {
      return  p ? new(p) ::caliHit : new ::caliHit;
   }
   static void *newArray_caliHit(Long_t nElements, void *p) {
      return p ? new(p) ::caliHit[nElements] : new ::caliHit[nElements];
   }
   // Wrapper around operator delete
   static void delete_caliHit(void *p) {
      delete ((::caliHit*)p);
   }
   static void deleteArray_caliHit(void *p) {
      delete [] ((::caliHit*)p);
   }
   static void destruct_caliHit(void *p) {
      typedef ::caliHit current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_caliHit(TBuffer &buf, void *obj) {
      ((::caliHit*)obj)->::caliHit::Streamer(buf);
   }
} // end of namespace ROOT for class ::caliHit

//______________________________________________________________________________
void caliCluster::Streamer(TBuffer &R__b)
{
   // Stream an object of class caliCluster.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      TObject::Streamer(R__b);
      R__b >> nhits;
      R__b >> x;
      R__b >> y;
      R__b >> z;
      R__b >> e;
      R__b.CheckByteCount(R__s, R__c, caliCluster::IsA());
   } else {
      R__c = R__b.WriteVersion(caliCluster::IsA(), kTRUE);
      TObject::Streamer(R__b);
      R__b << nhits;
      R__b << x;
      R__b << y;
      R__b << z;
      R__b << e;
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_caliCluster(void *p) {
      return  p ? new(p) ::caliCluster : new ::caliCluster;
   }
   static void *newArray_caliCluster(Long_t nElements, void *p) {
      return p ? new(p) ::caliCluster[nElements] : new ::caliCluster[nElements];
   }
   // Wrapper around operator delete
   static void delete_caliCluster(void *p) {
      delete ((::caliCluster*)p);
   }
   static void deleteArray_caliCluster(void *p) {
      delete [] ((::caliCluster*)p);
   }
   static void destruct_caliCluster(void *p) {
      typedef ::caliCluster current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_caliCluster(TBuffer &buf, void *obj) {
      ((::caliCluster*)obj)->::caliCluster::Streamer(buf);
   }
} // end of namespace ROOT for class ::caliCluster

namespace {
  void TriggerDictionaryInitialization_caliTypeDict_Impl() {
    static const char* headers[] = {
"caliType.h",
nullptr
    };
    static const char* includePaths[] = {
"/home/weibin/local/root/include/",
"/home/weibin/UCR/epic/BNL_test/include/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "caliTypeDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$caliType.h")))  caliHit;
class __attribute__((annotate("$clingAutoload$caliType.h")))  caliCluster;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "caliTypeDict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "caliType.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"caliCluster", payloadCode, "@",
"caliHit", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("caliTypeDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_caliTypeDict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_caliTypeDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_caliTypeDict() {
  TriggerDictionaryInitialization_caliTypeDict_Impl();
}
