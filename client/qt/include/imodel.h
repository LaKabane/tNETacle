#ifndef MODEL_H_
# define MODEL_H_


// This class is empty but shoudl allow us to add common features to mode objhects
// after disscution, toward communication with core features.
// (exemple: export in json features)

#include <QString>
#include <QMap>
#include <QByteArray>
#include <QVariant>

class IModel
{
public:
  const QByteArray toJson() const;//For debug only
  virtual const QString &getObjectName() const  = 0;
  virtual void  feedData(const QString &, const QVariant&) = 0;
  virtual const QMap<QString, QVariant>* getData() const = 0;
private:
  //  static const QMap<QString, QVariant> mapToVar(const QMap<QString, QString> &);
};

#endif /* !MODEL_H_ */
