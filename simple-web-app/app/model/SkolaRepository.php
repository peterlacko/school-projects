<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka user
 *   */
class SkolaRepository extends Repository
{
    /*public function fakultaOf(Nette\Database\Table\ActiveRow $skola)
    {
    	return $skola->related('fakulta')->order('Nazev_fakulty');
    }
     */
    public function findByName($nazev_skoly)
    {
	return $this->findAll()->where('Nazev_skoly', $nazev_skoly)->fetch();
    }

    public function addSkola($values)
    {
	$this->connection->table('skola')->insert(array(
	    'Nazev_skoly'=>$values->nazev_skoly,
	    'Adresa_skoly'=>$values->adresa_skoly,
	    'Email_skoly'=>$values->email_skoly,
	    'Web_skoly'=>$values->web_skoly,
	    'Rektor'=>$values->rektor,
	    'Nazev_zamereni'=>$values->zamereni
	));
	if ($values->logo_skoly != '') {
	    $logo = $values->logo_skoly->toImage();
	    $logo->save(__DIR__.'/../../www/images/logos/'.str_replace(' ', '_',$values->nazev_skoly).'.jpg', 98);
	}
    }

    public function getNamesAsArray()
    {
		$tmp = $this->getTable();
		return $tmp->fetchPairs('Nazev_skoly', 'Nazev_skoly');
    }

    public function deleteSkola($nazev)
    {
		$this->findBy(array('Nazev_skoly'=>$nazev))->delete();
    }

    public function search($phrase)
    {
		return $this->connection->table('skola')
			->where("Nazev_skoly LIKE ? OR Adresa_skoly LIKE ? OR Rektor LIKE ?", "%$phrase%", "%$phrase%", "%$phrase%");
    }
}
