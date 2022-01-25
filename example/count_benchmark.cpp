#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

using boost::multi_index_container;
using namespace boost::multi_index;

/* an employee record holds its ID, name and age */

struct employee
{
  long long   id;
  std::string name;
  int         age;

  employee(long long id_,std::string name_,int age_):id(id_),name(name_),age(age_){}
};

struct id{};
struct name{};
struct age{};

typedef multi_index_container<
  employee,
  indexed_by<
    ordered_unique<
      tag<id>,  BOOST_MULTI_INDEX_MEMBER(employee,long long,id)>,
    ordered_non_unique<
      tag<name>,BOOST_MULTI_INDEX_MEMBER(employee,std::string,name)>,
    ordered_non_unique<
      tag<age>, BOOST_MULTI_INDEX_MEMBER(employee,int,age)> >
> employee_set;

typedef multi_index_container<
  employee,
  indexed_by<
    ordered_unique<
      tag<id>,  BOOST_MULTI_INDEX_MEMBER(employee,long long,id)>,
    ordered_non_unique<
      tag<name>,BOOST_MULTI_INDEX_MEMBER(employee,std::string,name)>,
    ranked_non_unique<
      tag<age>, BOOST_MULTI_INDEX_MEMBER(employee,int,age)> >
> employee_ranked_set;

int main()
{
  employee_set es;
  employee_ranked_set ers;
  const int max_age=100;

  std::cout<<"This program benchmarks two implementations, and the duration "
             "of its execution may be considerable. Please feel free to stop "
             "it at any point or play with the constants.\n";

  std::cout<<"Running test for "<<max_age
           <<" different values of the index on which we call count.\n";

  int last_id1=0,last_id2=0;
  
  for(int j=-1;j<11;++j){
    int size=es.size();
    int people=1000;
    switch(j){
      case -1:
        people=100;
        break;
      case 0:
        people=900;
        break;
      case 10:
        people=90000;
        break;
    }
    std::cout<<"Adding "<<people<<" new people to the multi_index";
    for(int i=0;i<people;++i){
      es.insert(employee(last_id1++,"Joe",i%max_age));
      ers.insert(employee(last_id2++,"Joe",i%max_age));
      if((10*i)%people==0)
        std::cout<<"."<<std::flush;
    }
    std::cout<<std::endl;

    for(int iters=100;iters<=1000000;iters*=100){
      std::cout<<"Size "<<std::setw(6)<<es.size()<< ", "
               <<std::setw(7)<<iters<<" calls of count()";
      auto start=std::chrono::steady_clock::now();
      auto dur1=start-start;
      auto dur2=start-start;

      const int loops=std::sqrt(iters);
      for(int k=0;k<loops;++k){
        if((10*k)%loops==0)
          std::cout<<"."<<std::flush;

        auto start1=std::chrono::steady_clock::now();
        for(int i=0;i<iters/loops;++i) {
          int count=es.get<age>().count(i%max_age);
          if(count<1) // To prevent compiler optimisations.
            std::cout<<count<<std::endl;
        }
        auto end1=std::chrono::steady_clock::now();
        dur1+=end1-start1;

        auto start2=std::chrono::steady_clock::now();
        for(int i=0;i<iters/loops;++i){
          int count=ers.get<age>().count(i%max_age);
          if(count<1) // To prevent compiler optimisations.
            std::cout<<count<<std::endl;
        }
        auto end2=std::chrono::steady_clock::now();
        dur2+=end2-start2;

        // The following is aimed at avoiding the impact of caching.
        auto begin_index1=es.get<id>().begin();
        int removed_age1=begin_index1->age;
        es.get<id>().erase(begin_index1);
        es.insert(employee(last_id1++,"Anna",removed_age1));

        auto begin_index2=ers.get<id>().begin();
        int removed_age2=begin_index2->age;
        ers.get<id>().erase(begin_index2);
        ers.insert(employee(last_id2++,"Anna",removed_age2));
      }

      long long durMicro1=
        std::chrono::duration_cast<std::chrono::microseconds>(dur1).count();
      std::cout<<std::endl<<std::setw(20)<<durMicro1
               <<" - time of ordered_index.\n";
 
      long long durMicro2=
        std::chrono::duration_cast<std::chrono::microseconds>(dur2).count();
      std::cout<<std::setw(20)<<durMicro2
               <<" - time of ranked_index.\n";

      std::cout<<std::fixed<<std::setprecision(2)<<std::setw(20)
               <<((double)durMicro1)/durMicro2<<" - ratio."<<std::endl;
    }
  }

  return 0;
}
