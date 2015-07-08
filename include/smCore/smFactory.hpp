#ifndef SMFACTORY_HPP
#define SMFACTORY_HPP

#include <stdlib.h> // for atexit()

template<typename T>
void smFactory<T>::registerClassConfiguration(
  const std::string& classname,
  const std::string& subclassname,
  SharedPointerConstructor ctor,
  int group)
{
  if (classname.empty())
    return;

  if (!smFactory::s_catalog)
    {
    smFactory::s_catalog =
      new std::map<std::string, typename smFactory<T>::smFactoryConfigurationOptions>;
    atexit( []() { delete smFactory::s_catalog; } );
    }

  smFactoryEntry entry;
  entry.subclassname = subclassname;
  entry.constructor = ctor;
  entry.group = group;
  (*smFactory::s_catalog)[classname].insert(entry);
}

template<typename T>
const typename smFactory<T>::smFactoryConfigurationOptions& smFactory<T>::optionsForClass(const std::string& classname)
{
  static smFactoryConfigurationOptions emptyOptions;
  if (!smFactory::s_catalog)
    return emptyOptions;

  typename std::map<std::string, smFactoryConfigurationOptions>::const_iterator it;
  if (classname.empty() || (it = smFactory::s_catalog->find(classname)) == smFactory::s_catalog->end())
    return emptyOptions;
  return it->second;
}

template<typename T>
std::shared_ptr<T> smFactory<T>::createDefault(
  const std::string& classname)
{
  const smFactoryConfigurationOptions& opts(
    smFactory::optionsForClass(classname));
  if (opts.empty())
    return std::shared_ptr<T>();
  //std::cout << "Creating default " << classname << ", " << opts.begin()->subclassname << "\n";
  return opts.begin()->constructor();
}

template<typename T>
std::shared_ptr<T> smFactory<T>::createSubclass(
  const std::string& classname,
  const std::string& subclassname)
{
  const smFactoryConfigurationOptions& opts(
    smFactory::optionsForClass(classname));

  typename smFactoryConfigurationOptions::const_iterator it;
  for (it = opts.begin(); it != opts.end(); ++it)
    if (it->subclassname == subclassname)
      {
      //std::cout << "Creating " << it->subclassname << " (" << classname << ")\n";
      return it->constructor();
      }

  return std::shared_ptr<T>();
}

/**\brief Create an instance given the name of a concrete class.
  *
  * This method will be slow since the map of all abstract bases
  * must be traversed to find the constructor for the concrete
  * class.
  */
template<typename T>
std::shared_ptr<T> smFactory<T>::createConcreteClass(
  const std::string& classname)
{
  if (classname.empty() || !smFactory::s_catalog)
    return std::shared_ptr<T>();

  typename std::map<std::string, smFactoryConfigurationOptions>::const_iterator bit;
  typename smFactoryConfigurationOptions::const_iterator cit;
  for (bit = smFactory::s_catalog->begin(); bit != smFactory::s_catalog->end(); ++bit)
    for (cit = bit->second.begin(); cit != bit->second.end(); ++cit)
      if (cit->subclassname == classname)
        {
        //std::cout << "Creating " << cit->subclassname << " (" << classname << " implied)\n";
        return cit->constructor();
        }

  return std::shared_ptr<T>();
}

template<typename T>
std::shared_ptr<T> smFactory<T>::createSubclassForGroup(
  const std::string& classname,
  int group)
{
  const smFactoryConfigurationOptions& opts(
    smFactory::optionsForClass(classname));

  typename smFactoryConfigurationOptions::const_iterator it;
  for (it = opts.begin(); it != opts.end(); ++it)
    if (it->group == group)
      {
      //std::cout << "Creating " << it->subclassname << " (" << classname << ", " << it->group << ")\n";
      return it->constructor();
      }

  return std::shared_ptr<T>();
}

/// Class-static map from abstract class names to registered concrete children.
template<typename T>
std::map<std::string, typename smFactory<T>::smFactoryConfigurationOptions>* smFactory<T>::s_catalog = NULL;

#endif // SMFACTORY_HPP
