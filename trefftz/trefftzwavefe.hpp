#ifndef FILE_TREFFTZELEMENT_HPP
#define FILE_TREFFTZELEMENT_HPP

#include <fem.hpp>
#include "helpers.hpp"
#include "scalarmappedfe.hpp"

namespace ngfem
{
  typedef Vec<3, Vector<>> coo; // COO sparse matrix in (row,col,val) format

  template <int D> class TrefftzWaveFE : public ScalarMappedElement<D>
  {
  private:
    const int ord;
    const int nbasis;
    const int npoly;
    Vec<D> elcenter;
    double elsize;
    float c;
    ELEMENT_TYPE eltype;

  public:
    // TrefftzWaveFE();
    TrefftzWaveFE (int aord = 1, float ac = 1.0, Vec<D> aelcenter = 0,
                   double aelsize = 1, ELEMENT_TYPE aeltype = ET_TRIG);

    virtual ELEMENT_TYPE ElementType () const { return eltype; }

    using ScalarMappedElement<D>::CalcShape;
    virtual void CalcShape (const BaseMappedIntegrationPoint &mip,
                            BareSliceVector<> shape) const;
    virtual void CalcShape (const SIMD_MappedIntegrationRule<D - 1, D> &smir,
                            BareSliceMatrix<SIMD<double>> shape) const;

    using ScalarMappedElement<D>::CalcDShape;
    virtual void CalcDShape (const BaseMappedIntegrationPoint &mip,
                             SliceMatrix<> dshape) const;
    virtual void CalcDShape (const SIMD_MappedIntegrationRule<D - 1, D> &smir,
                             SliceMatrix<SIMD<double>> dshape) const;

    int GetNBasis () const { return nbasis; }
    float GetWavespeed () const { return c; }
    void SetWavespeed (double wavespeed) { c = wavespeed; }

    // TrefftzWaveFE<D> * SetCenter(Vec<D> acenter) {elcenter = acenter; return
    // this;} TrefftzWaveFE<D> * SetElSize(double aelsize) {elsize = aelsize;
    // return this;}
    //  TrefftzWaveFE<D> * SetWavespeed(float ac) {c = ac; return this;}

  protected:
    void MakeIndices_inner (Matrix<int> &indice, Vec<D, int> numbers,
                            int &count, int ordr, int dim) const;
    Matrix<int> MakeIndices () const;

    constexpr int IndexMap (Vec<D, int> index) const;
    Matrix<double> TrefftzBasis () const;
    Matrix<double> GetDerTrefftzBasis (int der) const;
    Matrix<int> pascal_sym () const;
  };

  class Monomial : public RecursivePolynomial<Monomial>
  {
  public:
    Monomial () { ; }

    template <class S, class T> inline Monomial (int n, S x, T &&values)
    {
      Eval (n, x, values);
    }

    template <class S> static INLINE double P0 (S x) { return 1.0; }
    template <class S> static INLINE S P1 (S x) { return x; }
    template <class S, class Sy> static INLINE S P1 (S x, Sy y)
    {
      return P1 (x);
    }

    static INLINE double A (int i) { return 1.0; }
    static INLINE double B (int i) { return 0; }
    static INLINE double C (int i) { return 0; }

    static INLINE double CalcA (int i) { return 1.0; }
    static INLINE double CalcB (int i) { return 0; }
    static INLINE double CalcC (int i) { return 0; }

    enum
    {
      ZERO_B = 1
    };
  };

  template <int D> class TrefftzWaveBasis
  {
  public:
    static TrefftzWaveBasis &getInstance ()
    {
      static TrefftzWaveBasis instance;
      // volatile int dummy{};
      return instance;
    }

    const coo *TB (int ord);

  private:
    TrefftzWaveBasis () = default;
    ~TrefftzWaveBasis () = default;
    TrefftzWaveBasis (const TrefftzWaveBasis &) = delete;
    TrefftzWaveBasis &operator= (const TrefftzWaveBasis &) = delete;

    mutex gentrefftzbasis;
    Array<coo> tbstore;
    // once_flag tbonceflag;
    void TB_inner (int ord, Matrix<> &trefftzbasis, Vec<D, int> coeffnum,
                   int basis, int dim, int &tracker);
    int IndexMap2 (Vec<D, int> index, int ord);

    void MatToCOO (Matrix<> mat, coo &sparsemat)
    {
      int nnonzero = 0;
      for (auto val : mat.AsVector ())
        if (val)
          ++nnonzero;
      sparsemat[0].SetSize (nnonzero);
      sparsemat[1].SetSize (nnonzero);
      sparsemat[2].SetSize (nnonzero);
      for (int i = 0, counter = 0; i < mat.Height (); i++)
        for (int j = 0; j < mat.Width (); j++)
          if (mat (i, j))
            {
              sparsemat[0][counter] = i;
              sparsemat[1][counter] = j;
              sparsemat[2][counter] = mat (i, j);
              counter++;
            }
    };
  };

}

#ifdef NGS_PYTHON
#include <python_ngstd.hpp>
void ExportTrefftzElement (py::module m);
#endif // NGS_PYTHON

#endif // FILE_TrefftzElement_HPP
